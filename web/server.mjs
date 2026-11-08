import {
  createHash,
  createHmac,
  randomBytes,
  randomInt,
  randomUUID,
  scrypt as scryptCallback,
  timingSafeEqual
} from "node:crypto";
import { promisify } from "node:util";
import { createServer } from "node:http";
import { spawn } from "node:child_process";
import { createInterface } from "node:readline";
import { existsSync, mkdirSync, readFileSync, renameSync, writeFileSync } from "node:fs";
import { dirname, join } from "node:path";
import { fileURLToPath, pathToFileURL } from "node:url";
import dotenv from "dotenv";
import nodemailer from "nodemailer";
import { WebSocket, WebSocketServer } from "ws";
import { containsBannedName } from "./name-policy.mjs";

const scrypt = promisify(scryptCallback);
const SESSION_COOKIE = "dungpoly_session";
const SESSION_TTL_MS = 7 * 24 * 60 * 60 * 1000;
const AUTH_CODE_TTL_MS = 10 * 60 * 1000;
const AUTH_CODE_ATTEMPTS = 5;
const MAX_BODY_BYTES = 16 * 1024;
const PASSWORD_HASH_LENGTH = 64;
const CURRENT_SCRYPT_OPTIONS = { N: 32768, r: 8, p: 1, maxmem: 64 * 1024 * 1024 };
const LEGACY_SCRYPT_OPTIONS = { N: 16384, r: 8, p: 1, maxmem: 32 * 1024 * 1024 };
const AUTH_WINDOW_MS = 15 * 60 * 1000;
const LOGIN_ATTEMPTS_PER_ACCOUNT = 10;
const LOGIN_ATTEMPTS_PER_IP = 30;
const REGISTRATIONS_PER_IP = 10;
const AUTH_REQUESTS_PER_IP = 60;
const MAX_RATE_LIMIT_BUCKETS = 10000;
const WEB_ROOT = fileURLToPath(new URL(".", import.meta.url));
dotenv.config({ path: join(WEB_ROOT, ".env") });
const PUBLIC_ROOT = join(WEB_ROOT, "public");
const RESOURCE_ROOT = join(WEB_ROOT, "..", "resources");
const STATIC_FILES = new Map([
  ["/", ["index.html", "text/html; charset=utf-8"]],
  ["/index.html", ["index.html", "text/html; charset=utf-8"]],
  ["/styles.css", ["styles.css", "text/css; charset=utf-8"]],
  ["/app.js", ["app.js", "text/javascript; charset=utf-8"]],
  ["/game.css", ["game.css", "text/css; charset=utf-8"]],
  ["/game.js", ["game.js", "text/javascript; charset=utf-8"]]
]);
const GAME_RESOURCES = [
  "BOOT.png", "BUY_BACK_PROPERTY.png", "BUY_FIELD.png", "BUY_HOTEL_RU.png",
  "BUY_HOUSE.png", "CAR.png", "CAT.png", "DOG.png", "END_TURN.png",
  "EXIT_GAME.png", "FINISH_GAME.png", "HAT.png", "MORTGAGE_HOLDINGS.png",
  "PAY_TAX_RU.png", "PAY_TO_OTHER_PLAYER.png", "ROLL_DICE.png",
  "SELL_FIELD_RU.png", "SELL_HOTEL_RU.png", "SELL_HOUSE.png", "SHIP.png",
  "START_TRADE.png", "START_TRADE_NEW_FIELD.png", "TAKE_CARD.png",
  "USE_CARD.png", "Ubuntu-R.ttf"
];
for (const file of GAME_RESOURCES) {
  const contentType = file.endsWith(".ttf") ? "font/ttf" : "image/png";
  STATIC_FILES.set(`/resources/${file}`, [file, contentType, RESOURCE_ROOT]);
}

class HttpError extends Error {
  constructor(status, message) {
    super(message);
    this.status = status;
  }
}

function readStore(filePath) {
  if (!existsSync(filePath)) {
    return { users: [], lobbies: [], sessions: [], authChallenges: [] };
  }

  const parsed = JSON.parse(readFileSync(filePath, "utf8"));
  if (!Array.isArray(parsed.users) || !Array.isArray(parsed.lobbies)) {
    throw new Error("Lobby store has an invalid shape");
  }
  parsed.sessions ??= [];
  parsed.authChallenges ??= [];
  if (!Array.isArray(parsed.sessions)) {
    throw new Error("Lobby store has an invalid sessions collection");
  }
  if (!Array.isArray(parsed.authChallenges)) {
    throw new Error("Lobby store has an invalid authentication challenges collection");
  }
  for (const lobby of parsed.lobbies) {
    if (lobby.gameStatus === "starting" || lobby.gameStatus === "playing") {
      lobby.gameStatus = "interrupted";
    }
    lobby.gameStatus ??= "waiting";
  }
  return parsed;
}

function saveStore(filePath, store) {
  mkdirSync(dirname(filePath), { recursive: true });
  const temporaryPath = `${filePath}.${randomUUID()}.tmp`;
  writeFileSync(temporaryPath, JSON.stringify(store), { mode: 0o600 });
  renameSync(temporaryPath, filePath);
}

function sendJson(response, status, value, headers = {}) {
  response.writeHead(status, {
    "Cache-Control": "no-store",
    "Content-Type": "application/json; charset=utf-8",
    ...headers
  });
  response.end(JSON.stringify(value));
}

function cookieValue(request, name) {
  const cookieHeader = request.headers.cookie ?? "";
  for (const part of cookieHeader.split(";")) {
    const separator = part.indexOf("=");
    if (separator < 0) continue;
    if (part.slice(0, separator).trim() === name) {
      return part.slice(separator + 1).trim();
    }
  }
  return undefined;
}

function sessionCookie(value, secure) {
  const secureAttribute = secure ? "; Secure" : "";
  return `${SESSION_COOKIE}=${value}; HttpOnly; SameSite=Strict; Path=/; Max-Age=${SESSION_TTL_MS / 1000}${secureAttribute}`;
}

function clearSessionCookie(secure) {
  const secureAttribute = secure ? "; Secure" : "";
  return `${SESSION_COOKIE}=; HttpOnly; SameSite=Strict; Path=/; Max-Age=0${secureAttribute}`;
}

function hashSessionToken(token) {
  return createHash("sha256").update(token).digest("hex");
}

function isValidPasswordHash(hash) {
  return typeof hash === "string" && /^[\da-f]{128}$/i.test(hash);
}

async function readJson(request) {
  let body = "";
  for await (const chunk of request) {
    body += chunk;
    if (Buffer.byteLength(body) > MAX_BODY_BYTES) {
      throw new HttpError(413, "Слишком большой запрос.");
    }
  }

  try {
    const parsed = body ? JSON.parse(body) : {};
    if (!parsed || typeof parsed !== "object" || Array.isArray(parsed)) {
      throw new HttpError(400, "Ожидается JSON-объект.");
    }
    return parsed;
  } catch (error) {
    if (error instanceof HttpError) throw error;
    throw new HttpError(400, "Некорректный JSON.");
  }
}

function requireSameOrigin(request) {
  const origin = request.headers.origin;
  if (!origin) {
    throw new HttpError(403, "Для этого действия нужен запрос с сайта DungPoly.");
  }

  let originUrl;
  const expectedOrigin = process.env.DUNGPOLY_PUBLIC_ORIGIN;
  try {
    originUrl = new URL(origin);
  } catch {
    throw new HttpError(403, "Недопустимый источник запроса.");
  }
  const expected = expectedOrigin
    ? new URL(expectedOrigin).origin
    : `${process.env.NODE_ENV === "production" ? "https" : "http"}://${request.headers.host}`;
  if (originUrl.origin !== expected || originUrl.username || originUrl.password) {
    throw new HttpError(403, "Запрос с другого сайта отклонён.");
  }
}

function publicUser(user) {
  return { id: user.id, username: user.username };
}

function findUser(store, userId) {
  return store.users.find((user) => user.id === userId);
}

function requireUser(request, store) {
  const token = cookieValue(request, SESSION_COOKIE);
  if (!token || !/^[A-Za-z0-9_-]{43}$/.test(token)) {
    throw new HttpError(401, "Войдите в аккаунт, чтобы продолжить.");
  }

  const tokenHash = hashSessionToken(token);
  const sessionIndex = store.sessions.findIndex((entry) => entry.tokenHash === tokenHash);
  const session = store.sessions[sessionIndex];
  if (!session || session.expiresAt <= Date.now()) {
    if (session) {
      store.sessions.splice(sessionIndex, 1);
      saveStore(store.dataFile, store);
    }
    throw new HttpError(401, "Войдите в аккаунт, чтобы продолжить.");
  }
  const user = findUser(store, session.userId);
  if (!user) {
    store.sessions.splice(sessionIndex, 1);
    saveStore(store.dataFile, store);
    throw new HttpError(401, "Сессия завершена. Войдите снова.");
  }
  return { user, token, session };
}

async function createPasswordHash(password, salt, options = CURRENT_SCRYPT_OPTIONS) {
  return Buffer.from(await scrypt(password, salt, PASSWORD_HASH_LENGTH, options)).toString("hex");
}

async function verifyPassword(user, password) {
  const legacyHash = !user.passwordKdf;
  const options = legacyHash ? LEGACY_SCRYPT_OPTIONS : CURRENT_SCRYPT_OPTIONS;
  if (!isValidPasswordHash(user.passwordHash)) {
    throw new Error(`Account ${user.id} has an invalid password hash`);
  }
  const supplied = Buffer.from(await createPasswordHash(password, user.passwordSalt, options), "hex");
  const stored = Buffer.from(user.passwordHash, "hex");
  return {
    valid: timingSafeEqual(stored, supplied),
    needsUpgrade: legacyHash
  };
}

function pruneRateLimits(rateLimits, now) {
  for (const [key, bucket] of rateLimits) {
    if (now - bucket.startedAt >= AUTH_WINDOW_MS) rateLimits.delete(key);
  }
  while (rateLimits.size >= MAX_RATE_LIMIT_BUCKETS) {
    const oldestKey = rateLimits.keys().next().value;
    if (oldestKey === undefined) break;
    rateLimits.delete(oldestKey);
  }
}

function takeRateLimit(rateLimits, key, limit) {
  const now = Date.now();
  let bucket = rateLimits.get(key);
  if (!bucket || now - bucket.startedAt >= AUTH_WINDOW_MS) {
    pruneRateLimits(rateLimits, now);
    bucket = { startedAt: now, attempts: 0 };
    rateLimits.set(key, bucket);
  }
  if (bucket.attempts >= limit) {
    const retryAfter = Math.max(1, Math.ceil((bucket.startedAt + AUTH_WINDOW_MS - now) / 1000));
    throw new HttpError(429, `Слишком много попыток. Повторите через ${retryAfter} сек.`);
  }
  bucket.attempts += 1;
}

function createSession(store, user) {
  const now = Date.now();
  store.sessions = store.sessions.filter((session) => session.expiresAt > now);
  const token = randomBytes(32).toString("base64url");
  store.sessions.push({
    tokenHash: hashSessionToken(token),
    userId: user.id,
    expiresAt: now + SESSION_TTL_MS,
    createdAt: new Date(now).toISOString()
  });
  saveStore(store.dataFile, store);
  return token;
}

function lobbyView(lobby, user) {
  return {
    id: lobby.id,
    name: lobby.name,
    owner: lobby.players.find((player) => player.userId === lobby.ownerUserId)?.username ?? "Игрок",
    players: lobby.players.map((player) => ({ username: player.username })),
    maxPlayers: lobby.maxPlayers,
    gameStatus: lobby.gameStatus ?? "waiting",
    createdAt: lobby.createdAt,
    isOwner: lobby.ownerUserId === user.id,
    isMember: lobby.players.some((player) => player.userId === user.id)
  };
}

function validateCredentials(input) {
  if (!input || typeof input.username !== "string" || typeof input.email !== "string") {
    throw new HttpError(400, "Укажите имя профиля и почту.");
  }
  const username = input.username.trim();
  if (!/^[\p{L}\p{N}_-]{3,24}$/u.test(username)) {
    throw new HttpError(400, "Имя: от 3 до 24 букв, цифр, дефисов или подчёркиваний.");
  }
  if (containsBannedName(username)) {
    throw new HttpError(400, "Имя профиля содержит запрещённое слово.");
  }
  const email = input.email.trim();
  const normalizedEmail = email.normalize("NFKC").toLowerCase();
  if (email.length > 254 || !/^[^\s@]+@[^\s@]+\.[^\s@]{2,}$/u.test(email)) {
    throw new HttpError(400, "Укажите корректный адрес электронной почты.");
  }

  return {
    username,
    normalizedUsername: username.normalize("NFKC").toLowerCase(),
    email,
    normalizedEmail
  };
}

function validateEmail(input) {
  if (!input || typeof input.email !== "string") {
    throw new HttpError(400, "Укажите адрес электронной почты.");
  }
  const email = input.email.trim();
  const normalizedEmail = email.normalize("NFKC").toLowerCase();
  if (email.length > 254 || !/^[^\s@]+@[^\s@]+\.[^\s@]{2,}$/u.test(email)) {
    throw new HttpError(400, "Укажите корректный адрес электронной почты.");
  }
  return { email, normalizedEmail };
}

async function sendVerificationEmail(email, code) {
  const from = process.env.GMAIL_USER?.trim();
  const appPassword = process.env.GMAIL_APP_PASSWORD?.replace(/\s/g, "");
  if (!from || !appPassword || appPassword.length !== 16) {
    throw new HttpError(503, "Отправка кодов не настроена: укажите GMAIL_USER и пароль приложения GMAIL_APP_PASSWORD.");
  }
  const transporter = nodemailer.createTransport({
    host: "smtp.gmail.com",
    port: 465,
    secure: true,
    connectionTimeout: 15000,
    greetingTimeout: 10000,
    socketTimeout: 20000,
    auth: { user: from, pass: appPassword }
  });
  try {
    await transporter.sendMail({
      from: { name: "DungPoly", address: from },
      to: email,
      subject: "Код входа в DungPoly",
      text: `Ваш код подтверждения DungPoly: ${code}\n\nКод действует 10 минут. Если вы не запрашивали его, просто проигнорируйте это письмо.`
    });
  } catch (error) {
    console.error(
      "Verification email delivery failed:",
      error instanceof Error ? (error.code ?? error.name) : "unknown"
    );
    throw new HttpError(502, "Не удалось отправить письмо. Проверьте настройки Gmail SMTP и повторите попытку.");
  } finally {
    transporter.close();
  }
}

export function createAppServer({
  dataFile = process.env.DUNGPOLY_DATA_FILE ?? join(WEB_ROOT, "data", "store.json"),
  sendEmail = sendVerificationEmail,
  verificationSecret = process.env.DUNGPOLY_OTP_SECRET
} = {}) {
  const store = readStore(dataFile);
  Object.defineProperty(store, "dataFile", { value: dataFile });
  const rateLimits = new Map();
  const gameSessions = new Map();
  const roomSockets = new Map();
  const secureCookies = process.env.NODE_ENV === "production";

  async function createAuthChallenge(details) {
    if (typeof verificationSecret !== "string" || Buffer.byteLength(verificationSecret) < 32) {
      throw new HttpError(503, "DUNGPOLY_OTP_SECRET должен содержать не менее 32 байт.");
    }

    const now = Date.now();
    const id = randomUUID();
    const code = String(randomInt(0, 1000000)).padStart(6, "0");
    const challenge = {
      id,
      ...details,
      codeHash: createHmac("sha256", verificationSecret).update(`${id}:${code}`).digest("hex"),
      attempts: 0,
      createdAt: new Date(now).toISOString(),
      expiresAt: now + AUTH_CODE_TTL_MS
    };
    store.authChallenges = store.authChallenges.filter((entry) =>
      entry.expiresAt > now && entry.normalizedEmail !== details.normalizedEmail
    );
    store.authChallenges.push(challenge);
    saveStore(dataFile, store);

    try {
      await sendEmail(challenge.email, code);
    } catch (error) {
      store.authChallenges = store.authChallenges.filter((entry) => entry.id !== id);
      saveStore(dataFile, store);
      throw error;
    }
    return id;
  }

  function verifyAuthChallenge(challengeId, code) {
    if (typeof challengeId !== "string" || typeof code !== "string" || !/^\d{6}$/.test(code)) {
      throw new HttpError(400, "Введите шестизначный код из письма.");
    }
    const challenge = store.authChallenges.find((entry) => entry.id === challengeId);
    if (!challenge || challenge.expiresAt <= Date.now()) {
      store.authChallenges = store.authChallenges.filter((entry) => entry.id !== challengeId);
      saveStore(dataFile, store);
      throw new HttpError(401, "Код неверный или истёк. Запросите новый.");
    }

    const supplied = Buffer.from(createHmac("sha256", verificationSecret)
      .update(`${challenge.id}:${code}`).digest("hex"), "hex");
    const expected = Buffer.from(challenge.codeHash, "hex");
    challenge.attempts += 1;
    if (!timingSafeEqual(supplied, expected)) {
      if (challenge.attempts >= AUTH_CODE_ATTEMPTS) {
        store.authChallenges = store.authChallenges.filter((entry) => entry.id !== challengeId);
      }
      saveStore(dataFile, store);
      throw new HttpError(401, "Код неверный или истёк. Запросите новый.");
    }
    store.authChallenges = store.authChallenges.filter((entry) => entry.id !== challengeId);
    return challenge;
  }

  function sendSocket(socket, message) {
    if (socket.readyState === WebSocket.OPEN) socket.send(JSON.stringify(message));
  }

  function broadcastRoom(lobbyId, message) {
    const sockets = roomSockets.get(lobbyId);
    if (!sockets) return;
    for (const socket of sockets) sendSocket(socket, message);
  }

  function notifyLobby(lobby) {
    for (const socket of roomSockets.get(lobby.id) ?? []) {
      const user = findUser(store, socket.userId);
      if (user) sendSocket(socket, { type: "lobby", lobby: lobbyView(lobby, user) });
    }
  }

  function stopGame(lobbyId) {
    const game = gameSessions.get(lobbyId);
    if (!game) return;
    gameSessions.delete(lobbyId);
    stopChildSession(game);
  }

  function enginePath() {
    const executable = process.platform === "win32" ? "DungPolyWebEngine.exe" : "DungPolyWebEngine";
    const candidates = [
      process.env.DUNGPOLY_ENGINE_PATH,
      join(WEB_ROOT, "bin", executable),
      join(WEB_ROOT, "..", "build", "win-msvc", "web-engine", "Debug", executable),
      join(WEB_ROOT, "..", "build", "win-msvc", "web-engine", "Release", executable),
      join(WEB_ROOT, "..", "build", "web-engine", "Debug", executable),
      join(WEB_ROOT, "..", "build", "web-engine", "Release", executable)
    ].filter(Boolean);
    return candidates.find((candidate) => existsSync(candidate));
  }

  function stopChildSession(session) {
    if (session.closed) return;
    session.closed = true;
    if (session.child.stdin.writable) session.child.stdin.write("STOP\n");
    session.child.kill();
  }

  function startEngine(lobby) {
    const executable = enginePath();
    if (!executable) {
      throw new HttpError(
        503,
        "Игровой сервер ещё не собран. Соберите цель DungPolyWebEngine через CMake."
      );
    }

    const tokenToUser = new Map();
    const playerData = [];
    lobby.players.forEach((player, index) => {
      const token = index + 1;
      tokenToUser.set(token, player.userId);
      playerData.push(String(token), Buffer.from(player.username, "utf8").toString("base64"));
    });

    const child = spawn(executable, [], { stdio: ["pipe", "pipe", "pipe"], windowsHide: true });
    const session = {
      child,
      lobbyId: lobby.id,
      tokenToUser,
      state: null,
      pending: null,
      closed: false,
      done: false
    };
    gameSessions.set(lobby.id, session);

    const output = createInterface({ input: child.stdout });
    output.on("line", (line) => {
      if (session.closed) return;
      const separator = line.indexOf("\t");
      if (separator < 0) {
        console.error("Invalid game engine output");
        return;
      }
      const kind = line.slice(0, separator);
      let payload;
      try {
        payload = JSON.parse(line.slice(separator + 1));
      } catch (error) {
        console.error("Invalid game engine JSON output:", error);
        broadcastRoom(lobby.id, { type: "game_error", message: "Игровой сервер прислал некорректное состояние." });
        return;
      }

      if (kind === "STATE") {
        session.state = payload;
        broadcastRoom(lobby.id, { type: "state", state: payload });
      } else if (kind === "REQUEST") {
        const userId = tokenToUser.get(payload.playerToken);
        if (!userId) {
          stopChildSession(session);
          broadcastRoom(lobby.id, { type: "game_error", message: "Не удалось определить игрока для хода." });
          return;
        }
        session.pending = { ...payload, userId };
        for (const socket of roomSockets.get(lobby.id) ?? []) {
          if (socket.userId === userId) {
            sendSocket(socket, { type: "request", request: payload });
          } else {
            const player = lobby.players.find((entry) => entry.userId === userId);
            sendSocket(socket, { type: "waiting", player: player?.username ?? "Игрок" });
          }
        }
      } else if (kind === "MESSAGE") {
        broadcastRoom(lobby.id, { type: "notification", message: payload.message, messageType: payload.messageType });
      } else if (kind === "REPLY_ERROR") {
        broadcastRoom(lobby.id, { type: "game_error", message: payload });
      } else if (kind === "DONE") {
        session.done = true;
        session.pending = null;
        lobby.gameStatus = "finished";
        saveStore(dataFile, store);
        notifyLobby(lobby);
        broadcastRoom(lobby.id, { type: "game_over", state: session.state });
        stopChildSession(session);
      } else if (kind === "ERROR") {
        lobby.gameStatus = "failed";
        saveStore(dataFile, store);
        notifyLobby(lobby);
        broadcastRoom(lobby.id, { type: "game_error", message: payload });
        stopChildSession(session);
      }
    });

    child.stderr.on("data", (chunk) => {
      console.error(`Game engine ${lobby.id}: ${chunk.toString("utf8").trim()}`);
    });
    child.on("error", (error) => {
      session.closed = true;
      gameSessions.delete(lobby.id);
      lobby.gameStatus = "failed";
      saveStore(dataFile, store);
      notifyLobby(lobby);
      broadcastRoom(lobby.id, { type: "game_error", message: "Не удалось запустить игровой процесс." });
      console.error("Failed to start game engine:", error);
    });
    child.on("close", (code) => {
      output.close();
      if (!session.done && !session.closed && lobby.gameStatus === "playing") {
        lobby.gameStatus = "failed";
        saveStore(dataFile, store);
        notifyLobby(lobby);
        broadcastRoom(lobby.id, {
          type: "game_error",
          message: `Игровой процесс завершился неожиданно (код ${code ?? "неизвестен"}).`
        });
      }
      if (gameSessions.get(lobby.id) === session) gameSessions.delete(lobby.id);
    });

    child.stdin.write(`START\t${playerData.join("\t")}\n`);
    lobby.gameStatus = "playing";
    saveStore(dataFile, store);
    notifyLobby(lobby);
    broadcastRoom(lobby.id, { type: "game_started" });
    return session;
  }

  function handleGameMessage(socket, rawMessage) {
    if (rawMessage.length > 4096) {
      socket.close(1009, "Message too large");
      return;
    }
    let message;
    try {
      message = JSON.parse(rawMessage.toString());
    } catch {
      sendSocket(socket, { type: "error", message: "Некорректное сообщение." });
      return;
    }
    const session = gameSessions.get(socket.lobbyId);
    const pending = session?.pending;
    if (!message || typeof message !== "object" || Array.isArray(message) ||
        !pending || pending.userId !== socket.userId || message.type !== "reply" ||
        message.requestId !== pending.id) {
      sendSocket(socket, { type: "error", message: "Сейчас нельзя отправить этот ответ." });
      return;
    }
    const replyType = message.replyType;
    const value = message.value;
    if (typeof replyType !== "string" || typeof value !== "string" || value.length > 32) {
      sendSocket(socket, { type: "error", message: "Недопустимый ответ." });
      return;
    }

    const validTypes = {
      ACTION: pending.type === "ACTION" && pending.actions.includes(value),
      TRADE: pending.type === "TRADE" && ["PARTICIPATE", "REFUSE"].includes(value),
      NUM: pending.type === "NUM" && /^(?:[0-9]|[12][0-9]|3[0-9])$/.test(value),
      SUM: pending.type === "SUM" && /^(?:0|[1-9]\d{0,8})$/.test(value),
      TOKEN: pending.type === "TOKEN" && /^[1-6]$/.test(value)
    };
    if (!validTypes[replyType]) {
      sendSocket(socket, { type: "error", message: "Ответ не соответствует текущему запросу." });
      return;
    }

    session.pending = null;
    session.child.stdin.write(`REPLY\t${pending.id}\t${replyType}\t${value}\n`);
  }

  const websocketServer = new WebSocketServer({ noServer: true, maxPayload: 4096, perMessageDeflate: false });

  const server = createServer(async (request, response) => {
    try {
      const requestUrl = new URL(request.url ?? "/", `http://${request.headers.host ?? "localhost"}`);
      const pathname = requestUrl.pathname;

      if (pathname.startsWith("/api/")) {
        response.setHeader("X-Content-Type-Options", "nosniff");

        if (request.method === "GET" && pathname === "/api/me") {
          const { user } = requireUser(request, store);
          sendJson(response, 200, { user: publicUser(user) });
          return;
        }

        if (request.method === "POST") {
          requireSameOrigin(request);
          const body = await readJson(request);

          if (pathname === "/api/auth/register") {
            const credentials = validateCredentials(body);
            const ip = request.socket.remoteAddress ?? "unknown";
            takeRateLimit(rateLimits, `register:${ip}`, REGISTRATIONS_PER_IP);
            if (store.users.some((user) =>
              (user.normalizedUsername ?? user.username.normalize("NFKC").toLowerCase()) === credentials.normalizedUsername
            )) {
              throw new HttpError(409, "Это имя пользователя уже занято.");
            }
            if (store.users.some((user) => user.normalizedEmail === credentials.normalizedEmail)) {
              throw new HttpError(409, "Эта почта уже привязана к профилю.");
            }
            takeRateLimit(rateLimits, `auth-request-ip:${ip}`, AUTH_REQUESTS_PER_IP);
            takeRateLimit(rateLimits, `auth-request-email:${credentials.normalizedEmail}`, 4);
            const challengeId = await createAuthChallenge({
              purpose: "register",
              username: credentials.username,
              normalizedUsername: credentials.normalizedUsername,
              email: credentials.email,
              normalizedEmail: credentials.normalizedEmail
            });
            sendJson(response, 202, { challengeId, message: "Код отправлен на указанную почту." });
            return;
          }

          if (pathname === "/api/auth/login") {
            const { email, normalizedEmail } = validateEmail(body);
            const ip = request.socket.remoteAddress ?? "unknown";
            takeRateLimit(rateLimits, `auth-request-ip:${ip}`, AUTH_REQUESTS_PER_IP);
            takeRateLimit(rateLimits, `auth-request-email:${normalizedEmail}`, 4);
            const user = store.users.find((candidate) =>
              candidate.normalizedEmail === normalizedEmail && candidate.emailVerifiedAt
            );
            const challengeId = user
              ? await createAuthChallenge({
                purpose: "login",
                userId: user.id,
                email,
                normalizedEmail
              })
              : randomUUID();
            sendJson(response, 202, {
              challengeId,
              message: "Если почта зарегистрирована, на неё отправлен код для входа."
            });
            return;
          }

          if (pathname === "/api/auth/migrate") {
            if (typeof body.username !== "string" || typeof body.password !== "string" ||
                body.password.length < 1 || body.password.length > 128) {
              throw new HttpError(400, "Укажите имя старого профиля и его пароль.");
            }
            const normalizedUsername = body.username.trim().normalize("NFKC").toLowerCase();
            const ip = request.socket.remoteAddress ?? "unknown";
            takeRateLimit(rateLimits, `login-ip:${ip}`, LOGIN_ATTEMPTS_PER_IP);
            takeRateLimit(rateLimits, `login-user:${normalizedUsername}`, LOGIN_ATTEMPTS_PER_ACCOUNT);
            const user = store.users.find((candidate) =>
              (candidate.normalizedUsername ?? candidate.username.normalize("NFKC").toLowerCase()) === normalizedUsername
            );
            const hasPassword = Boolean(user?.passwordHash);
            const hashRecord = hasPassword ? user : {
              id: "unknown",
              passwordSalt: "00000000000000000000000000000000",
              passwordHash: Buffer.alloc(PASSWORD_HASH_LENGTH).toString("hex"),
              passwordKdf: "scrypt-32768-8-1"
            };
            const verification = await verifyPassword(hashRecord, body.password);
            if (!user) {
              throw new HttpError(401, "Не удалось подтвердить старый профиль.");
            }
            if (!hasPassword) {
              throw new HttpError(409, "Этот профиль уже перенесён. Войдите по почте.");
            }
            if (!verification.valid) {
              throw new HttpError(401, "Не удалось подтвердить старый профиль.");
            }
            const newProfile = validateCredentials({
              username: typeof body.profileName === "string" && body.profileName.trim()
                ? body.profileName
                : user.username,
              email: body.email
            });
            if (store.users.some((candidate) =>
              candidate.id !== user.id && candidate.normalizedEmail === newProfile.normalizedEmail
            )) {
              throw new HttpError(409, "Эта почта уже привязана к профилю.");
            }
            if (store.users.some((candidate) =>
              candidate.id !== user.id &&
              (candidate.normalizedUsername ?? candidate.username.normalize("NFKC").toLowerCase()) === newProfile.normalizedUsername
            )) {
              throw new HttpError(409, "Это имя пользователя уже занято.");
            }
            takeRateLimit(rateLimits, `auth-request-ip:${ip}`, AUTH_REQUESTS_PER_IP);
            takeRateLimit(rateLimits, `auth-request-email:${newProfile.normalizedEmail}`, 4);
            const challengeId = await createAuthChallenge({
              purpose: "migrate",
              userId: user.id,
              username: newProfile.username,
              normalizedUsername: newProfile.normalizedUsername,
              email: newProfile.email,
              normalizedEmail: newProfile.normalizedEmail
            });
            sendJson(response, 202, { challengeId, message: "Код отправлен на указанную почту." });
            return;
          }

          if (pathname === "/api/auth/verify") {
            const ip = request.socket.remoteAddress ?? "unknown";
            takeRateLimit(rateLimits, `auth-verify-ip:${ip}`, 30);
            const challenge = verifyAuthChallenge(body.challengeId, body.code);
            const emailInUse = store.users.some((user) =>
              user.id !== challenge.userId && user.normalizedEmail === challenge.normalizedEmail
            );
            if (emailInUse) {
              saveStore(dataFile, store);
              throw new HttpError(409, "Эта почта уже привязана к другому профилю.");
            }

            let user;
            if (challenge.purpose === "register") {
              const usernameInUse = store.users.some((entry) =>
                (entry.normalizedUsername ?? entry.username.normalize("NFKC").toLowerCase()) === challenge.normalizedUsername
              );
              if (usernameInUse) {
                saveStore(dataFile, store);
                throw new HttpError(409, "Это имя пользователя уже занято.");
              }
              user = {
                id: randomUUID(),
                username: challenge.username,
                normalizedUsername: challenge.normalizedUsername,
                email: challenge.email,
                normalizedEmail: challenge.normalizedEmail,
                emailVerifiedAt: new Date().toISOString(),
                createdAt: new Date().toISOString()
              };
              store.users.push(user);
            } else {
              user = store.users.find((entry) => entry.id === challenge.userId);
              if (!user || (challenge.purpose !== "login" && challenge.purpose !== "migrate")) {
                saveStore(dataFile, store);
                throw new HttpError(401, "Код неверный или истёк. Запросите новый.");
              }
              if (challenge.purpose === "login" &&
                  (user.normalizedEmail !== challenge.normalizedEmail || !user.emailVerifiedAt)) {
                saveStore(dataFile, store);
                throw new HttpError(401, "Код неверный или истёк. Запросите новый.");
              }
              if (challenge.purpose === "migrate" && !user.passwordHash) {
                saveStore(dataFile, store);
                throw new HttpError(401, "Код неверный или истёк. Запросите новый.");
              }
              if (challenge.purpose === "migrate") {
                const usernameInUse = store.users.some((entry) =>
                  entry.id !== user.id &&
                  (entry.normalizedUsername ?? entry.username.normalize("NFKC").toLowerCase()) === challenge.normalizedUsername
                );
                if (usernameInUse) {
                  saveStore(dataFile, store);
                  throw new HttpError(409, "Это имя пользователя уже занято.");
                }
              }
              user.email = challenge.email;
              user.normalizedEmail = challenge.normalizedEmail;
              user.emailVerifiedAt ??= new Date().toISOString();
              if (challenge.purpose === "migrate") {
                user.username = challenge.username;
                user.normalizedUsername = challenge.normalizedUsername;
                delete user.passwordSalt;
                delete user.passwordHash;
                delete user.passwordKdf;
                store.authChallenges = store.authChallenges.filter((entry) => entry.userId !== user.id);
              }
            }
            const token = createSession(store, user);
            sendJson(response, 200, { user: publicUser(user) }, { "Set-Cookie": sessionCookie(token, secureCookies) });
            return;
          }

          if (pathname === "/api/register" || pathname === "/api/login") {
            throw new HttpError(410, "Вход по паролю отключён. Используйте подтверждение почты.");
          }

          if (pathname === "/api/logout") {
            const token = cookieValue(request, SESSION_COOKIE);
            if (token) {
              const tokenHash = hashSessionToken(token);
              store.sessions = store.sessions.filter((session) => session.tokenHash !== tokenHash);
              saveStore(dataFile, store);
            }
            sendJson(response, 200, { ok: true }, { "Set-Cookie": clearSessionCookie(secureCookies) });
            return;
          }

          if (pathname === "/api/lobbies") {
            const { user } = requireUser(request, store);
            if (typeof body.name !== "string") {
              throw new HttpError(400, "Укажите название лобби.");
            }
            const name = body.name.trim();
            if (name.length < 3 || name.length > 40) {
              throw new HttpError(400, "Название лобби должно содержать от 3 до 40 символов.");
            }
            if (containsBannedName(name)) {
              throw new HttpError(400, "Название лобби содержит запрещённое слово.");
            }
            if (!Number.isInteger(body.maxPlayers) || body.maxPlayers < 2 || body.maxPlayers > 6) {
              throw new HttpError(400, "В лобби должно быть от 2 до 6 игроков.");
            }

            let id;
            do {
              id = randomBytes(4).toString("hex").toUpperCase();
            } while (store.lobbies.some((lobby) => lobby.id === id));
            const lobby = {
              id,
              name,
              ownerUserId: user.id,
              maxPlayers: body.maxPlayers,
              gameStatus: "waiting",
              createdAt: new Date().toISOString(),
              players: [{ userId: user.id, username: user.username, joinedAt: new Date().toISOString() }]
            };
            store.lobbies.push(lobby);
            saveStore(dataFile, store);
            sendJson(response, 201, { lobby: lobbyView(lobby, user) });
            return;
          }

          const startRoute = pathname.match(/^\/api\/lobbies\/([A-F0-9]{8})\/start$/);
          if (startRoute) {
            const { user } = requireUser(request, store);
            const lobby = store.lobbies.find((candidate) => candidate.id === startRoute[1]);
            if (!lobby) throw new HttpError(404, "Лобби не найдено.");
            if (lobby.ownerUserId !== user.id) {
              throw new HttpError(403, "Начать игру может только владелец лобби.");
            }
            if (lobby.players.length < 2) {
              throw new HttpError(409, "Для начала игры нужны как минимум два игрока.");
            }
            if (lobby.players.length > 6) {
              throw new HttpError(409, "В игре может быть не более шести игроков.");
            }
            if (!["waiting", "interrupted", "failed"].includes(lobby.gameStatus)) {
              throw new HttpError(409, "Эта партия уже начата или завершена.");
            }
            if (gameSessions.has(lobby.id)) {
              throw new HttpError(409, "Игровой процесс этой комнаты уже запущен.");
            }
            startEngine(lobby);
            sendJson(response, 202, { lobby: lobbyView(lobby, user) });
            return;
          }
        }

        if (request.method === "GET" && pathname === "/api/lobbies") {
          const { user } = requireUser(request, store);
          const lobbies = [...store.lobbies]
            .sort((left, right) => right.createdAt.localeCompare(left.createdAt))
            .map((lobby) => lobbyView(lobby, user));
          sendJson(response, 200, { lobbies });
          return;
        }

        const lobbyRoute = pathname.match(/^\/api\/lobbies\/([A-F0-9]{8})(?:\/(join|leave))?$/);
        if (lobbyRoute && request.method === "POST") {
          requireSameOrigin(request);
          await readJson(request);
          const { user } = requireUser(request, store);
          const [, lobbyId, action] = lobbyRoute;
          const lobby = store.lobbies.find((candidate) => candidate.id === lobbyId);
          if (!lobby) throw new HttpError(404, "Лобби не найдено.");

          if (action === "join") {
            if (lobby.players.some((player) => player.userId === user.id)) {
              sendJson(response, 200, { lobby: lobbyView(lobby, user) });
              return;
            }
            if (lobby.gameStatus !== "waiting") {
              throw new HttpError(409, "В эту партию уже нельзя войти.");
            }
            if (lobby.players.length >= lobby.maxPlayers) {
              throw new HttpError(409, "В этом лобби уже нет свободных мест.");
            }
            lobby.players.push({ userId: user.id, username: user.username, joinedAt: new Date().toISOString() });
            saveStore(dataFile, store);
            notifyLobby(lobby);
            sendJson(response, 200, { lobby: lobbyView(lobby, user) });
            return;
          }

          if (action === "leave") {
            if (lobby.gameStatus === "playing") {
              throw new HttpError(409, "Нельзя покинуть лобби во время партии.");
            }
            lobby.players = lobby.players.filter((player) => player.userId !== user.id);
            if (lobby.players.length === 0) {
              stopGame(lobby.id);
              store.lobbies = store.lobbies.filter((candidate) => candidate.id !== lobby.id);
            } else if (lobby.ownerUserId === user.id) {
              lobby.ownerUserId = lobby.players[0].userId;
            }
            saveStore(dataFile, store);
            notifyLobby(lobby);
            sendJson(response, 200, { ok: true });
            return;
          }
        }

        if (lobbyRoute && request.method === "DELETE") {
          requireSameOrigin(request);
          const { user } = requireUser(request, store);
          const lobby = store.lobbies.find((candidate) => candidate.id === lobbyRoute[1]);
          if (!lobby) throw new HttpError(404, "Лобби не найдено.");
          if (lobby.ownerUserId !== user.id) throw new HttpError(403, "Удалить лобби может только его владелец.");
          stopGame(lobby.id);
          for (const socket of roomSockets.get(lobby.id) ?? []) socket.close(4004, "Lobby closed");
          roomSockets.delete(lobby.id);
          store.lobbies = store.lobbies.filter((candidate) => candidate.id !== lobby.id);
          saveStore(dataFile, store);
          sendJson(response, 200, { ok: true });
          return;
        }

        throw new HttpError(404, "API-маршрут не найден.");
      }

      if (request.method !== "GET" && request.method !== "HEAD") {
        throw new HttpError(405, "Метод не поддерживается.");
      }
      const roomPage = pathname.match(/^\/lobby\/([A-F0-9]{8})$/);
      if (roomPage) {
        const { user } = requireUser(request, store);
        const lobby = store.lobbies.find((candidate) => candidate.id === roomPage[1]);
        if (!lobby) throw new HttpError(404, "Лобби не найдено.");
        if (!lobby.players.some((player) => player.userId === user.id)) {
          throw new HttpError(403, "Вы не состоите в этом лобби.");
        }
        const content = readFileSync(join(PUBLIC_ROOT, "game.html"));
        response.writeHead(200, {
          "Cache-Control": "no-store",
          "Content-Security-Policy": "default-src 'self'; script-src 'self'; style-src 'self'; img-src 'self' data:; font-src 'self'; connect-src 'self' ws: wss:; frame-ancestors 'none'",
          "Content-Type": "text/html; charset=utf-8",
          "Cross-Origin-Opener-Policy": "same-origin",
          "Permissions-Policy": "camera=(), microphone=(), geolocation=()",
          "Referrer-Policy": "no-referrer",
          "X-Frame-Options": "DENY",
          "X-Content-Type-Options": "nosniff"
        });
        response.end(request.method === "HEAD" ? undefined : content);
        return;
      }

      const staticFile = STATIC_FILES.get(pathname);
      if (!staticFile) throw new HttpError(404, "Страница не найдена.");
      const [fileName, contentType, fileRoot = PUBLIC_ROOT] = staticFile;
      const content = readFileSync(join(fileRoot, fileName));
      response.writeHead(200, {
        "Cache-Control": "no-cache",
        "Content-Security-Policy": "default-src 'self'; script-src 'self'; style-src 'self'; img-src 'self' data:; font-src 'self'; connect-src 'self' ws: wss:; frame-ancestors 'none'",
        "Content-Type": contentType,
        "Cross-Origin-Opener-Policy": "same-origin",
        "Permissions-Policy": "camera=(), microphone=(), geolocation=()",
        "Referrer-Policy": "no-referrer",
        "X-Frame-Options": "DENY",
        "X-Content-Type-Options": "nosniff"
      });
      response.end(request.method === "HEAD" ? undefined : content);
    } catch (error) {
      if (response.headersSent) {
        response.destroy(error);
        return;
      }
      if (error instanceof HttpError) {
        sendJson(response, error.status, { error: error.message });
        return;
      }
      console.error("DungPoly web request failed:", error);
      sendJson(response, 500, { error: "Внутренняя ошибка сервера." });
    }
  });

  server.on("upgrade", (request, socket, head) => {
    try {
      const requestUrl = new URL(request.url ?? "/", `http://${request.headers.host ?? "localhost"}`);
      if (requestUrl.pathname !== "/api/game") throw new HttpError(404, "WebSocket-маршрут не найден.");
      requireSameOrigin(request);
      const { user } = requireUser(request, store);
      const lobbyId = requestUrl.searchParams.get("lobby");
      const lobby = store.lobbies.find((candidate) => candidate.id === lobbyId);
      if (!lobby) throw new HttpError(404, "Лобби не найдено.");
      if (!lobby.players.some((player) => player.userId === user.id)) {
        throw new HttpError(403, "Вы не состоите в этом лобби.");
      }

      websocketServer.handleUpgrade(request, socket, head, (websocket) => {
        websocket.userId = user.id;
        websocket.lobbyId = lobby.id;
        const sockets = roomSockets.get(lobby.id) ?? new Set();
        sockets.add(websocket);
        roomSockets.set(lobby.id, sockets);
        websocketServer.emit("connection", websocket, request);

        sendSocket(websocket, { type: "lobby", lobby: lobbyView(lobby, user) });
        const session = gameSessions.get(lobby.id);
        if (session?.state) sendSocket(websocket, { type: "state", state: session.state });
        if (session?.pending) {
          if (session.pending.userId === user.id) {
            const { userId: _userId, ...publicRequest } = session.pending;
            sendSocket(websocket, { type: "request", request: publicRequest });
          } else {
            const currentPlayer = lobby.players.find((player) => player.userId === session.pending.userId);
            sendSocket(websocket, { type: "waiting", player: currentPlayer?.username ?? "Игрок" });
          }
        }
      });
    } catch (error) {
      const status = error instanceof HttpError ? error.status : 500;
      const message = error instanceof HttpError ? error.message : "Ошибка подключения.";
      socket.write(`HTTP/1.1 ${status} Error\r\nConnection: close\r\nContent-Type: text/plain; charset=utf-8\r\n\r\n${message}`);
      socket.destroy();
    }
  });

  websocketServer.on("connection", (websocket) => {
    websocket.on("message", (data) => handleGameMessage(websocket, data));
    websocket.on("error", (error) => console.error("Game WebSocket error:", error));
    websocket.on("close", () => {
      const sockets = roomSockets.get(websocket.lobbyId);
      sockets?.delete(websocket);
      if (sockets?.size === 0) roomSockets.delete(websocket.lobbyId);
    });
  });

  server.on("close", () => {
    for (const lobbyId of gameSessions.keys()) stopGame(lobbyId);
    websocketServer.close();
  });

  return server;
}

if (process.argv[1] && import.meta.url === pathToFileURL(process.argv[1]).href) {
  const port = Number(process.env.PORT ?? 3000);
  const host = process.env.HOST ?? "127.0.0.1";
  const server = createAppServer();
  server.listen(port, host, () => {
    console.log(`DungPoly lobby is listening at http://${host}:${port}`);
  });
}
