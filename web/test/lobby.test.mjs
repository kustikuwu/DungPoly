import assert from "node:assert/strict";
import { existsSync } from "node:fs";
import { mkdtemp, readFile, rm, writeFile } from "node:fs/promises";
import { tmpdir } from "node:os";
import { join } from "node:path";
import { after, before, test } from "node:test";
import { fileURLToPath } from "node:url";
import { runInNewContext } from "node:vm";
import { WebSocket } from "ws";
import { onRequest as proxyRequest } from "../functions/_proxy.js";
import { createAppServer } from "../server.mjs";

let server;
let baseUrl;
let dataDirectory;
let persistentSessionCookie;
const emailCodes = new Map();
const registeredEmails = new Map();
const registeredCookies = new Map();
let emailSequence = 0;

function createTestServer(dataFile) {
  return createAppServer({
    dataFile,
    verificationSecret: "test-only-verification-secret-0123456789",
    sendEmail: async (email, code) => emailCodes.set(email, code)
  });
}

before(async () => {
  dataDirectory = await mkdtemp(join(tmpdir(), "dungpoly-lobby-test-"));
  server = createTestServer(join(dataDirectory, "store.json"));
  await new Promise((resolve) => server.listen(0, "127.0.0.1", resolve));
  baseUrl = `http://127.0.0.1:${server.address().port}`;
});

after(async () => {
  await new Promise((resolve, reject) => server.close((error) => error ? reject(error) : resolve()));
  await rm(dataDirectory, { recursive: true, force: true });
});

function originHeaders(cookie) {
  return {
    Origin: baseUrl,
    ...(cookie ? { Cookie: cookie } : {})
  };
}

async function request(path, { method = "GET", cookie, body } = {}) {
  const response = await fetch(`${baseUrl}${path}`, {
    method,
    headers: {
      ...originHeaders(cookie),
      ...(body !== undefined ? { "Content-Type": "application/json" } : {})
    },
    ...(body !== undefined ? { body: JSON.stringify(body) } : {})
  });
  const payload = await response.json();
  const setCookie = response.headers.get("set-cookie");
  return {
    status: response.status,
    payload,
    cookie: setCookie?.split(";", 1)[0]
  };
}

async function register(username) {
  const email = `player-${++emailSequence}@example.test`;
  const challenge = await request("/api/auth/register", {
    method: "POST",
    body: { username, email }
  });
  assert.equal(challenge.status, 202, JSON.stringify(challenge.payload));
  const response = await request("/api/auth/verify", {
    method: "POST",
    body: { challengeId: challenge.payload.challengeId, code: emailCodes.get(email) }
  });
  assert.equal(response.status, 200, JSON.stringify(response.payload));
  registeredEmails.set(username, email);
  registeredCookies.set(username, response.cookie);
  return response.cookie;
}

function watchSocket(socket) {
  const messages = [];
  const waiters = [];
  socket.on("message", (data) => {
    const message = JSON.parse(data.toString());
    messages.push(message);
    for (let index = waiters.length - 1; index >= 0; index -= 1) {
      if (waiters[index].predicate(message)) {
        const [waiter] = waiters.splice(index, 1);
        clearTimeout(waiter.timeout);
        waiter.resolve(message);
      }
    }
  });
  return {
    waitFor(predicate, timeoutMs = 10000) {
      const existing = messages.find(predicate);
      if (existing) return Promise.resolve(existing);
      return new Promise((resolve, reject) => {
        const waiter = {
          predicate,
          resolve,
          timeout: setTimeout(() => {
            const index = waiters.indexOf(waiter);
            if (index >= 0) waiters.splice(index, 1);
            const received = messages.map((message) =>
              message.type === "request"
                ? `request:${message.request.type}[${message.request.actions?.join(",") ?? ""}]`
                : message.type
            ).join(", ");
            reject(new Error(`Timed out waiting for a WebSocket game event. Received: ${received}`));
          }, timeoutMs)
        };
        waiters.push(waiter);
      });
    }
  };
}

function connectToGame(cookie, lobbyId) {
  const socket = new WebSocket(`${baseUrl.replace(/^http/, "ws")}/api/game?lobby=${lobbyId}`, {
    headers: { Cookie: cookie, Origin: baseUrl }
  });
  const events = watchSocket(socket);
  return new Promise((resolve, reject) => {
    socket.once("open", () => resolve({ socket, events }));
    socket.once("error", reject);
  });
}

test("lobby endpoints require an authenticated account", async () => {
  const response = await request("/api/lobbies", {
    method: "POST",
    body: { name: "Открытый стол", maxPlayers: 4 }
  });
  assert.equal(response.status, 401);
});

test("mutating requests reject missing or cross-origin origins", async () => {
  const missingOrigin = await fetch(`${baseUrl}/api/auth/register`, {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ username: "Неториджина", email: "origin@example.test" })
  });
  assert.equal(missingOrigin.status, 403);

  const crossOrigin = await fetch(`${baseUrl}/api/auth/register`, {
    method: "POST",
    headers: { "Content-Type": "application/json", Origin: "https://evil.example" },
    body: JSON.stringify({ username: "Чужойисточник", email: "cross-origin@example.test" })
  });
  assert.equal(crossOrigin.status, 403);
});

test("the browser lobby page is served with restrictive security headers", async () => {
  const response = await fetch(baseUrl);
  assert.equal(response.status, 200);
  assert.match(response.headers.get("content-security-policy"), /default-src 'self'/);
  assert.equal(response.headers.get("referrer-policy"), "no-referrer");
  const html = await response.text();
  assert.match(html, /Создать лобби/);
  assert.doesNotMatch(html, /Старый аккаунт|Пароль старого аккаунта/);
});

test("the Pages proxy preserves API path, query, method, and session cookie", async () => {
  const originalFetch = globalThis.fetch;
  let upstreamRequest;
  globalThis.fetch = async (request) => {
    upstreamRequest = request;
    return new Response("upstream response", { status: 200 });
  };
  try {
    const response = await proxyRequest({
      request: new Request("https://dungpoly.pages.dev/api/me?check=1", {
        method: "POST",
        headers: {
          Cookie: "dungpoly_session=test-session",
          Origin: "https://dungpoly.pages.dev",
          "Content-Type": "application/json"
        },
        body: JSON.stringify({ ok: true })
      }),
      env: { DUNGPOLY_BACKEND_ORIGIN: "https://backend.example.test" }
    });
    assert.equal(response.status, 200);
    assert.equal(upstreamRequest.url, "https://backend.example.test/api/me?check=1");
    assert.equal(upstreamRequest.method, "POST");
    assert.equal(upstreamRequest.headers.get("cookie"), "dungpoly_session=test-session");
    assert.equal(await upstreamRequest.text(), JSON.stringify({ ok: true }));
  } finally {
    globalThis.fetch = originalFetch;
  }
});

test("authenticated players can create and join a lobby; capacity and ownership are enforced", async () => {
  const ownerCookie = await register("Создатель");
  persistentSessionCookie = ownerCookie;
  const guestCookie = await register("Гость");

  const guestLobby = await request("/api/lobbies", {
    method: "POST",
    cookie: guestCookie,
    body: { name: "Комната гостя", maxPlayers: 3 }
  });
  assert.equal(guestLobby.status, 201);
  assert.equal(guestLobby.payload.lobby.isOwner, true);

  const created = await request("/api/lobbies", {
    method: "POST",
    cookie: ownerCookie,
    body: { name: "Вечерняя партия", maxPlayers: 2 }
  });
  assert.equal(created.status, 201, JSON.stringify(created.payload));
  assert.equal(created.payload.lobby.isOwner, true);
  assert.equal(created.payload.lobby.players.length, 1);
  const { id } = created.payload.lobby;

  await request(`/api/lobbies/${guestLobby.payload.lobby.id}`, {
    method: "DELETE",
    cookie: guestCookie
  });

  const joined = await request(`/api/lobbies/${id}/join`, {
    method: "POST",
    cookie: guestCookie,
    body: {}
  });
  assert.equal(joined.status, 200);
  assert.equal(joined.payload.lobby.players.length, 2);

  const thirdCookie = await register("Третий");
  const fullJoin = await request(`/api/lobbies/${id}/join`, {
    method: "POST",
    cookie: thirdCookie,
    body: {}
  });
  assert.equal(fullJoin.status, 409, "a non-member cannot exceed the room capacity");

  const ownerPage = await fetch(`${baseUrl}/lobby/${id}`, {
    headers: originHeaders(ownerCookie)
  });
  assert.equal(ownerPage.status, 200);
  assert.match(ownerPage.headers.get("content-security-policy"), /connect-src 'self' ws: wss:/);
  assert.match(await ownerPage.text(), /id="game-board"/);
  const guestPage = await fetch(`${baseUrl}/lobby/${id}`, {
    headers: originHeaders(guestCookie)
  });
  assert.equal(guestPage.status, 200);
  const nonMemberPage = await request(`/lobby/${id}`, { cookie: thirdCookie });
  assert.equal(nonMemberPage.status, 403);
  const unauthenticatedPage = await request(`/lobby/${id}`);
  assert.equal(unauthenticatedPage.status, 401);

  const gameScript = await fetch(`${baseUrl}/game.js`);
  assert.equal(gameScript.status, 200);
  assert.match(gameScript.headers.get("content-type"), /javascript/);
  const gameSource = await gameScript.text();
  assert.match(gameSource, /tileIllustration/);
  assert.match(gameSource, /"Сдаться"/);
  assert.doesNotMatch(gameSource, /Завершить партию/);
  const actionImages = [...gameSource.matchAll(/"([A-Z][A-Z_]*\.png)"/g)]
    .map(([, image]) => image);
  const publicResources = fileURLToPath(new URL("../public/resources/", import.meta.url));
  assert.ok(actionImages.length > 0);
  assert.ok(actionImages.every((image) => existsSync(join(publicResources, image))));
  const servedActionImages = await Promise.all([...new Set(actionImages)].map((image) =>
    fetch(`${baseUrl}/resources/${image}`)
  ));
  assert.ok(servedActionImages.every((response) =>
    response.status === 200 && response.headers.get("content-type") === "image/png"
  ));
  const tileArtSource = gameSource.slice(gameSource.indexOf("const TILE_GLYPHS ="), gameSource.indexOf("const ACTIONS ="));
  const { TILE_ART, TILE_GLYPHS } = runInNewContext(`${tileArtSource}; ({ TILE_ART, TILE_GLYPHS });`);
  assert.equal(TILE_ART.length, 40);
  assert.ok(TILE_ART.every((glyph) => TILE_GLYPHS[glyph]));
  const tokenImage = await fetch(`${baseUrl}/resources/DOG.png`);
  assert.equal(tokenImage.status, 200);
  assert.match(tokenImage.headers.get("content-type"), /image\/png/);

  const guestDelete = await request(`/api/lobbies/${id}`, {
    method: "DELETE",
    cookie: guestCookie
  });
  assert.equal(guestDelete.status, 403);

  const listed = await request("/api/lobbies", { cookie: guestCookie });
  assert.equal(listed.status, 200);
  assert.equal(listed.payload.lobbies[0].isMember, true);
  assert.equal(listed.payload.lobbies[0].isOwner, false);

  const deleted = await request(`/api/lobbies/${id}`, {
    method: "DELETE",
    cookie: ownerCookie
  });
  assert.equal(deleted.status, 200);
  const emptyList = await request("/api/lobbies", { cookie: guestCookie });
  assert.deepEqual(emptyList.payload.lobbies, []);
});

test("the lobby starts a C++ game and routes a turn only to the active player", async (context) => {
  const engineName = process.platform === "win32" ? "DungPolyWebEngine.exe" : "DungPolyWebEngine";
  const projectRoot = fileURLToPath(new URL("../../", import.meta.url));
  const engineCandidates = ["Debug", "Release"].map((configuration) =>
    join(projectRoot, "build", "win-msvc", "web-engine", configuration, engineName)
  );
  if (!engineCandidates.some(existsSync)) {
    context.skip("Build the DungPolyWebEngine CMake target to run the native game integration test");
    return;
  }

  const ownerCookie = await register("ВладелецПартии");
  const guestCookie = await register("ВторойИгрок");
  const created = await request("/api/lobbies", {
    method: "POST",
    cookie: ownerCookie,
    body: { name: "Проверка движка", maxPlayers: 2 }
  });
  const lobbyId = created.payload.lobby.id;
  const joined = await request(`/api/lobbies/${lobbyId}/join`, {
    method: "POST",
    cookie: guestCookie,
    body: {}
  });
  assert.equal(joined.status, 200);

  const nonOwnerStart = await request(`/api/lobbies/${lobbyId}/start`, {
    method: "POST",
    cookie: guestCookie,
    body: {}
  });
  assert.equal(nonOwnerStart.status, 403);

  const ownerConnection = await connectToGame(ownerCookie, lobbyId);
  const guestConnection = await connectToGame(guestCookie, lobbyId);
  try {
    await Promise.all([
      ownerConnection.events.waitFor((message) => message.type === "lobby"),
      guestConnection.events.waitFor((message) => message.type === "lobby")
    ]);

    const started = await request(`/api/lobbies/${lobbyId}/start`, {
      method: "POST",
      cookie: ownerCookie,
      body: {}
    });
    assert.equal(started.status, 202, JSON.stringify(started.payload));

    const turn = await ownerConnection.events.waitFor((message) => message.type === "request");
    assert.equal(turn.request.type, "ACTION");
    assert.equal(turn.request.playerToken, 1);
    assert.equal(turn.request.actions.includes("ROLL_DICE"), true);

    guestConnection.socket.send(JSON.stringify({
      type: "reply",
      requestId: turn.request.id,
      replyType: "ACTION",
      value: "ROLL_DICE"
    }));
    const deniedReply = await guestConnection.events.waitFor((message) => message.type === "error");
    assert.match(deniedReply.message, /нельзя отправить/);

    ownerConnection.socket.send(JSON.stringify({
      type: "reply",
      requestId: turn.request.id,
      replyType: "ACTION",
      value: "NOT_AN_ACTION"
    }));
    const invalidReply = await ownerConnection.events.waitFor((message) => message.type === "error");
    assert.match(invalidReply.message, /не соответствует/);

    ownerConnection.socket.send(JSON.stringify({
      type: "reply",
      requestId: turn.request.id,
      replyType: "ACTION",
      value: "ROLL_DICE"
    }));
    const movedState = await ownerConnection.events.waitFor((message) =>
      message.type === "state" && message.state.players[0].position !== 0
    );
    assert.equal(movedState.state.players.length, 2);
    assert.equal(movedState.state.tiles.length, 40);
    assert.equal(Object.hasOwn(movedState.state.tiles[0], "shortName"), true);

    const lobbyView = await request("/api/lobbies", { cookie: ownerCookie });
    assert.equal(lobbyView.payload.lobbies[0].gameStatus, "playing");
  } finally {
    await request(`/api/lobbies/${lobbyId}`, {
      method: "DELETE",
      cookie: ownerCookie
    });
    ownerConnection.socket.close();
    guestConnection.socket.close();
  }
});

test("surrender removes a player and automatically finishes the game when one remains", async (context) => {
  const engineName = process.platform === "win32" ? "DungPolyWebEngine.exe" : "DungPolyWebEngine";
  const projectRoot = fileURLToPath(new URL("../../", import.meta.url));
  const engineCandidates = ["Debug", "Release"].map((configuration) =>
    join(projectRoot, "build", "win-msvc", "web-engine", configuration, engineName)
  );
  if (!engineCandidates.some(existsSync)) {
    context.skip("Build the DungPolyWebEngine CMake target to run the native game integration test");
    return;
  }

  const ownerCookie = await register("СдающийсяИгрок");
  const guestCookie = await register("ОставшийсяИгрок");
  const created = await request("/api/lobbies", {
    method: "POST",
    cookie: ownerCookie,
    body: { name: "Проверка сдачи", maxPlayers: 2 }
  });
  const lobbyId = created.payload.lobby.id;
  await request(`/api/lobbies/${lobbyId}/join`, {
    method: "POST",
    cookie: guestCookie,
    body: {}
  });

  const ownerConnection = await connectToGame(ownerCookie, lobbyId);
  const guestConnection = await connectToGame(guestCookie, lobbyId);
  try {
    await Promise.all([
      ownerConnection.events.waitFor((message) => message.type === "lobby"),
      guestConnection.events.waitFor((message) => message.type === "lobby")
    ]);

    const started = await request(`/api/lobbies/${lobbyId}/start`, {
      method: "POST",
      cookie: ownerCookie,
      body: {}
    });
    assert.equal(started.status, 202, JSON.stringify(started.payload));

    const surrenderTurn = await ownerConnection.events.waitFor((message) =>
      message.type === "request" && message.request.type === "ACTION"
    );
    assert.equal(surrenderTurn.request.actions.includes("EXIT_GAME"), true);
    assert.equal(surrenderTurn.request.actions.includes("FINISH_GAME"), false);
    ownerConnection.socket.send(JSON.stringify({
      type: "reply",
      requestId: surrenderTurn.request.id,
      replyType: "ACTION",
      value: "EXIT_GAME"
    }));

    const surrenderNotice = await ownerConnection.events.waitFor((message) =>
      message.type === "notification" && message.message === "Вы сдались"
    );
    assert.equal(surrenderNotice.messageType, 0);
    const [ownerGameOver, guestGameOver] = await Promise.all([
      ownerConnection.events.waitFor((message) => message.type === "game_over"),
      guestConnection.events.waitFor((message) => message.type === "game_over")
    ]);
    assert.equal(ownerGameOver.state.players[0].alive, false);
    assert.equal(ownerGameOver.state.players[1].alive, true);
    assert.equal(guestGameOver.state.players[0].alive, false);

    const finishedLobby = await request("/api/lobbies", { cookie: ownerCookie });
    assert.equal(finishedLobby.payload.lobbies[0].gameStatus, "finished");
  } finally {
    await request(`/api/lobbies/${lobbyId}`, {
      method: "DELETE",
      cookie: ownerCookie
    });
    ownerConnection.socket.close();
    guestConnection.socket.close();
  }
});

test("the owner can restart a game interrupted by a server restart", async (context) => {
  const engineName = process.platform === "win32" ? "DungPolyWebEngine.exe" : "DungPolyWebEngine";
  const projectRoot = fileURLToPath(new URL("../../", import.meta.url));
  const engineCandidates = ["Debug", "Release"].map((configuration) =>
    join(projectRoot, "build", "win-msvc", "web-engine", configuration, engineName)
  );
  if (!engineCandidates.some(existsSync)) {
    context.skip("Build the DungPolyWebEngine CMake target to run the native game integration test");
    return;
  }

  const isolatedDirectory = await mkdtemp(join(tmpdir(), "dungpoly-interrupted-test-"));
  const storeFile = join(isolatedDirectory, "store.json");
  let isolatedServer;
  let isolatedBaseUrl;
  let ownerCookie;
  let lobbyId;
  const startServer = async () => {
    isolatedServer = createTestServer(storeFile);
    await new Promise((resolve) => isolatedServer.listen(0, "127.0.0.1", resolve));
    isolatedBaseUrl = `http://127.0.0.1:${isolatedServer.address().port}`;
  };
  const closeServer = async () => {
    if (isolatedServer?.listening) {
      await new Promise((resolve, reject) =>
        isolatedServer.close((error) => error ? reject(error) : resolve())
      );
    }
  };
  const isolatedRequest = async (path, { method = "GET", cookie, body } = {}) => {
    const response = await fetch(`${isolatedBaseUrl}${path}`, {
      method,
      headers: {
        Origin: isolatedBaseUrl,
        ...(cookie ? { Cookie: cookie } : {}),
        ...(body !== undefined ? { "Content-Type": "application/json" } : {})
      },
      ...(body !== undefined ? { body: JSON.stringify(body) } : {})
    });
    const payload = await response.json();
    return {
      status: response.status,
      payload,
      cookie: response.headers.get("set-cookie")?.split(";", 1)[0]
    };
  };

  try {
    await startServer();
    const ownerEmail = `restart-owner-${++emailSequence}@example.test`;
    const ownerRequest = await isolatedRequest("/api/auth/register", {
      method: "POST",
      body: { username: "InterruptedOwner", email: ownerEmail }
    });
    assert.equal(ownerRequest.status, 202);
    const owner = await isolatedRequest("/api/auth/verify", {
      method: "POST",
      body: { challengeId: ownerRequest.payload.challengeId, code: emailCodes.get(ownerEmail) }
    });
    assert.equal(owner.status, 200);
    ownerCookie = owner.cookie;
    const guestEmail = `restart-guest-${++emailSequence}@example.test`;
    const guestRequest = await isolatedRequest("/api/auth/register", {
      method: "POST",
      body: { username: "InterruptedGuest", email: guestEmail }
    });
    assert.equal(guestRequest.status, 202);
    const guest = await isolatedRequest("/api/auth/verify", {
      method: "POST",
      body: { challengeId: guestRequest.payload.challengeId, code: emailCodes.get(guestEmail) }
    });
    assert.equal(guest.status, 200);
    const created = await isolatedRequest("/api/lobbies", {
      method: "POST",
      cookie: ownerCookie,
      body: { name: "Прерванная партия", maxPlayers: 2 }
    });
    assert.equal(created.status, 201);
    lobbyId = created.payload.lobby.id;
    const joined = await isolatedRequest(`/api/lobbies/${lobbyId}/join`, {
      method: "POST",
      cookie: guest.cookie,
      body: {}
    });
    assert.equal(joined.status, 200);

    const savedStore = JSON.parse(await readFile(storeFile, "utf8"));
    savedStore.lobbies[0].gameStatus = "playing";
    await writeFile(storeFile, JSON.stringify(savedStore));
    await closeServer();

    await startServer();
    const roomPage = await fetch(`${isolatedBaseUrl}/lobby/${lobbyId}`, {
      headers: { Cookie: ownerCookie }
    });
    assert.equal(roomPage.status, 200);
    const rooms = await isolatedRequest("/api/lobbies", { cookie: ownerCookie });
    assert.equal(rooms.payload.lobbies[0].gameStatus, "interrupted");

    const restarted = await isolatedRequest(`/api/lobbies/${lobbyId}/start`, {
      method: "POST",
      cookie: ownerCookie,
      body: {}
    });
    assert.equal(restarted.status, 202, JSON.stringify(restarted.payload));
  } finally {
    try {
      if (lobbyId && ownerCookie && isolatedServer?.listening) {
        await isolatedRequest(`/api/lobbies/${lobbyId}`, {
          method: "DELETE",
          cookie: ownerCookie
        });
      }
    } finally {
      await closeServer();
      await rm(isolatedDirectory, { recursive: true, force: true });
    }
  }
});

test("session cookies are backed by hashed tokens and survive a server restart", async () => {
  const savedStore = JSON.parse(await readFile(join(dataDirectory, "store.json"), "utf8"));
  const rawToken = persistentSessionCookie.split("=")[1];
  assert.equal(savedStore.sessions.length > 0, true);
  assert.equal(savedStore.sessions.some((session) => session.tokenHash === rawToken), false);
  assert.match(savedStore.sessions[0].tokenHash, /^[\da-f]{64}$/);

  const restartedServer = createTestServer(join(dataDirectory, "store.json"));
  await new Promise((resolve) => restartedServer.listen(0, "127.0.0.1", resolve));
  try {
    const restartedUrl = `http://127.0.0.1:${restartedServer.address().port}`;
    const response = await fetch(`${restartedUrl}/api/me`, {
      headers: { Cookie: persistentSessionCookie }
    });
    assert.equal(response.status, 200);
    assert.equal((await response.json()).user.username, "Создатель");
  } finally {
    await new Promise((resolve, reject) =>
      restartedServer.close((error) => error ? reject(error) : resolve())
    );
  }

  const loggedOut = await request("/api/logout", {
    method: "POST",
    cookie: persistentSessionCookie,
    body: {}
  });
  assert.equal(loggedOut.status, 200);
  const revokedSession = await request("/api/me", { cookie: persistentSessionCookie });
  assert.equal(revokedSession.status, 401);
});

test("email registration and login verify one-time codes; legacy account routes are disabled", async () => {
  const email = registeredEmails.get("Создатель");
  const challenge = await request("/api/auth/login", {
    method: "POST",
    body: { email }
  });
  assert.equal(challenge.status, 202);
  const login = await request("/api/auth/verify", {
    method: "POST",
    body: { challengeId: challenge.payload.challengeId, code: emailCodes.get(email) }
  });
  assert.equal(login.status, 200, JSON.stringify(login.payload));
  assert.equal(login.payload.user.username, "Создатель");

  const passwordLogin = await request("/api/login", {
    method: "POST",
    body: { username: "Создатель", password: "old-password" }
  });
  assert.equal(passwordLogin.status, 410);

  const passwordRegistration = await request("/api/register", {
    method: "POST",
    body: { username: "ЕщеОдин", password: "old-password" }
  });
  assert.equal(passwordRegistration.status, 410);

  const legacyMigration = await request("/api/auth/migrate", {
    method: "POST",
    body: { username: "Создатель", password: "old-password", email: "legacy@example.test" }
  });
  assert.equal(legacyMigration.status, 410);
});

test("a one-time code is invalidated after five incorrect attempts", async () => {
  const email = `attempt-limit-${++emailSequence}@example.test`;
  const requested = await request("/api/auth/register", {
    method: "POST",
    body: { username: "ПятьПопыток", email }
  });
  assert.equal(requested.status, 202);
  const validCode = emailCodes.get(email);
  const wrongCode = validCode === "000000" ? "000001" : "000000";
  for (let attempt = 0; attempt < 5; attempt += 1) {
    const invalid = await request("/api/auth/verify", {
      method: "POST",
      body: { challengeId: requested.payload.challengeId, code: wrongCode }
    });
    assert.equal(invalid.status, 401);
  }
  const invalidated = await request("/api/auth/verify", {
    method: "POST",
    body: { challengeId: requested.payload.challengeId, code: validCode }
  });
  assert.equal(invalidated.status, 401);
});

test("registration and lobby names reject profanity, malformed email, and duplicate usernames", async () => {
  const invalidEmail = await request("/api/auth/register", {
    method: "POST",
    body: { username: "НовыйИгрок", email: "not-an-email" }
  });
  assert.equal(invalidEmail.status, 400);

  const bannedUsername = await request("/api/auth/register", {
    method: "POST",
    body: { username: "f-u-c-k-player", email: "banned@example.test" }
  });
  assert.equal(bannedUsername.status, 400);

  for (const username of ["NaziPropaganda", "RacismClub", "СексуальноеНасилие", "ТорговляДетьми"]) {
    const response = await request("/api/auth/register", {
      method: "POST",
      body: { username, email: `${username}@example.test` }
    });
    assert.equal(response.status, 400, `${username} should be rejected`);
  }

  const mixedScriptUsername = await request("/api/auth/register", {
    method: "POST",
    body: { username: "СyкаИгрок", email: "mixed-script@example.test" }
  });
  assert.equal(mixedScriptUsername.status, 400);

  const duplicate = await request("/api/auth/register", {
    method: "POST",
    body: { username: "создатель", email: "another@example.test" }
  });
  assert.equal(duplicate.status, 409);

  const ownerCookie = await register("БезБрани");
  const bannedRoom = await request("/api/lobbies", {
    method: "POST",
    cookie: ownerCookie,
    body: { name: "fuck room", maxPlayers: 4 }
  });
  assert.equal(bannedRoom.status, 400);
  const bannedRussianRoom = await request("/api/lobbies", {
    method: "POST",
    cookie: ownerCookie,
    body: { name: "Комната Сyки", maxPlayers: 4 }
  });
  assert.equal(bannedRussianRoom.status, 400);

  for (const name of ["Комната для травли", "Terrorist Propaganda", "Незаконные азартные игры"]) {
    const response = await request("/api/lobbies", {
      method: "POST",
      cookie: ownerCookie,
      body: { name, maxPlayers: 4 }
    });
    assert.equal(response.status, 400, `${name} should be rejected`);
  }
});

test("three players finish a match after an auction leaves a property owner at zero balance", { timeout: 30000 }, async () => {
  const users = ["ВладелецПартии", "ВторойИгрок", "СдающийсяИгрок"]
    .map((username) => registeredCookies.get(username));
  assert.ok(users.every(Boolean), "the integration test accounts should already exist");
  const created = await request("/api/lobbies", {
    method: "POST",
    cookie: users[0],
    body: { name: "Офлайн-проверка аукциона", maxPlayers: 3 }
  });
  const lobbyId = created.payload.lobby.id;
  for (const cookie of users.slice(1)) {
    const joined = await request(`/api/lobbies/${lobbyId}/join`, {
      method: "POST",
      cookie,
      body: {}
    });
    assert.equal(joined.status, 200);
  }

  const connections = await Promise.all(users.map((cookie) => connectToGame(cookie, lobbyId)));
  let state;
  let rolls = 0;
  let auctions = 0;
  let zeroBalancePropertyOwner = false;
  let zeroBalancePlayerTookAnotherTurn = false;
  let forceAuction = true;
  let activeAuction = false;
  let bidPlaced = false;
  let timeoutId;
  let resolveGame;
  let rejectGame;
  const finished = new Promise((resolve, reject) => {
    resolveGame = resolve;
    rejectGame = reject;
  });
  const respond = (socket, gameRequest, replyType, value) => {
    socket.send(JSON.stringify({
      type: "reply",
      requestId: gameRequest.id,
      replyType,
      value: String(value)
    }));
  };

  for (const { socket } of connections) {
    socket.on("message", (data) => {
      const event = JSON.parse(data.toString());
      if (event.type === "state") {
        state = event.state;
        const bidder = state.players.find((player) => player.token === 2);
        zeroBalancePropertyOwner ||= bidder.money === 0 && bidder.alive &&
          state.tiles.some((tile) => tile.owner === bidder.token);
        if (zeroBalancePropertyOwner) activeAuction = false;
      } else if (event.type === "game_error") {
        rejectGame(new Error(event.message));
      } else if (event.type === "game_over") {
        resolveGame(event.state);
      }
    });
  }

  const respondToRequest = (socket, gameRequest) => {
    const player = state?.players.find((entry) => entry.token === gameRequest.playerToken);
    if (gameRequest.type === "ACTION") {
      const actions = gameRequest.actions;
      let action;
      if (rolls >= 40 && player.token !== 1 && actions.includes("EXIT_GAME")) {
        action = "EXIT_GAME";
      } else if (actions.includes("ROLL_DICE")) {
        if (player.token === 2 && zeroBalancePropertyOwner && player.money === 0) {
          zeroBalancePlayerTookAnotherTurn = true;
        }
        rolls += 1;
        action = "ROLL_DICE";
      } else if (actions.includes("BUY_FIELD") && forceAuction) {
        forceAuction = false;
        activeAuction = true;
        auctions += 1;
        action = "START_TRADE_NEW_FIELD";
      } else if (actions.includes("BUY_FIELD") && player.money >= state.tiles[player.position].cost) {
        action = "BUY_FIELD";
      } else if (
        (/недостаточно/i.test(gameRequest.message ?? "") ||
          (actions.includes("PAY_TAX") && player.money === 0)) &&
        actions.includes("MORTGAGE_HOLDINGS")
      ) {
        action = "MORTGAGE_HOLDINGS";
      } else if (actions.includes("PAY_TO_OTHER_PLAYER") && player.money > 0) {
        action = "PAY_TO_OTHER_PLAYER";
      } else if (actions.includes("PAY_TAX") && player.money > 0) {
        action = "PAY_TAX";
      } else if (actions.includes("END_TURN")) {
        action = "END_TURN";
      } else if (actions.includes("EXIT_GAME")) {
        action = "EXIT_GAME";
      } else if (actions.includes("START_TRADE_NEW_FIELD")) {
        action = "START_TRADE_NEW_FIELD";
      } else if (actions.includes("TAKE_CARD")) {
        action = "TAKE_CARD";
      } else {
        action = actions[0];
      }
      respond(socket, gameRequest, "ACTION", action);
    } else if (gameRequest.type === "TRADE") {
      const shouldBid = activeAuction && gameRequest.playerToken === 2 && !bidPlaced;
      respond(socket, gameRequest, "TRADE", shouldBid ? "PARTICIPATE" : "REFUSE");
    } else if (gameRequest.type === "SUM") {
      bidPlaced = true;
      respond(socket, gameRequest, "SUM", player.money);
    } else if (gameRequest.type === "NUM") {
      const ownUnbuiltTile = state.tiles.find((tile) =>
        tile.owner === player.token && !tile.mortgaged &&
        !(tile.houses ?? 0) && !(tile.hotels ?? 0)
      );
      respond(socket, gameRequest, "NUM", ownUnbuiltTile?.position ?? 0);
    } else if (gameRequest.type === "TOKEN") {
      respond(socket, gameRequest, "TOKEN", gameRequest.playerToken === 1 ? 2 : 1);
    }
  };
  for (const { socket, events } of connections) {
    socket.on("message", (data) => {
      const event = JSON.parse(data.toString());
      if (event.type === "request") respondToRequest(socket, event.request);
    });
    await events.waitFor((event) => event.type === "lobby");
  }

  try {
    const started = await request(`/api/lobbies/${lobbyId}/start`, {
      method: "POST",
      cookie: users[0],
      body: {}
    });
    assert.equal(started.status, 202);
    const timeout = new Promise((_, reject) => {
      timeoutId = setTimeout(() => reject(new Error("offline match timed out")), 25000);
    });
    const finalState = await Promise.race([finished, timeout]);
    clearTimeout(timeoutId);
    assert.equal(auctions, 1);
    assert.equal(zeroBalancePropertyOwner, true);
    assert.equal(zeroBalancePlayerTookAnotherTurn, true);
    assert.equal(finalState.players.filter((player) => player.alive).length, 1);
    assert.ok(finalState.players.find((player) => player.alive));
  } finally {
    clearTimeout(timeoutId);
    await request(`/api/lobbies/${lobbyId}`, {
      method: "DELETE",
      cookie: users[0]
    });
    for (const { socket } of connections) socket.close();
  }
});
