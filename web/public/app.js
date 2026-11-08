const currentUrl = new URL(window.location.href);
let removedSensitiveQuery = false;
for (const key of ["username", "password", "token", "access_token", "auth", "session"]) {
  if (currentUrl.searchParams.has(key)) {
    currentUrl.searchParams.delete(key);
    removedSensitiveQuery = true;
  }
}
if (removedSensitiveQuery) {
  window.history.replaceState(null, "", `${currentUrl.pathname}${currentUrl.search}${currentUrl.hash}`);
}

const authView = document.querySelector("#auth-view");
const lobbyView = document.querySelector("#lobby-view");
const authForm = document.querySelector("#auth-form");
const createForm = document.querySelector("#create-form");
const authError = document.querySelector("#auth-error");
const createError = document.querySelector("#create-error");
const authTitle = document.querySelector("#auth-title");
const authSubmit = document.querySelector("#auth-submit");
const credentialsFields = document.querySelector("#credentials-fields");
const verificationFields = document.querySelector("#verification-fields");
const verificationMessage = document.querySelector("#verification-message");
const usernameInput = document.querySelector("#username");
const usernameLabel = document.querySelector("#username-label");
const emailInput = document.querySelector("#email");
const verificationCodeInput = document.querySelector("#verification-code");
const accountBar = document.querySelector("#account-bar");
const currentUser = document.querySelector("#current-user");
const roomsList = document.querySelector("#rooms-list");
const roomsEmpty = document.querySelector("#rooms-empty");
const lobbyCount = document.querySelector("#lobby-count");
const statusMessage = document.querySelector("#status-message");

let authMode = "login";
let activeUser = null;
let refreshTimer = null;
let statusTimer = null;
let pendingChallengeId = null;

async function api(path, options = {}) {
  const response = await fetch(path, {
    credentials: "same-origin",
    ...options,
    headers: {
      ...(options.body ? { "Content-Type": "application/json" } : {}),
      ...options.headers
    }
  });
  const payload = await response.json().catch(() => ({}));
  if (!response.ok) {
    const error = new Error(payload.error || "Не удалось выполнить запрос.");
    error.status = response.status;
    throw error;
  }
  return payload;
}

function setFormError(element, message = "") {
  element.textContent = message;
  element.hidden = !message;
}

function showStatus(message) {
  statusMessage.textContent = message;
  statusMessage.hidden = false;
  clearTimeout(statusTimer);
  statusTimer = setTimeout(() => {
    statusMessage.hidden = true;
  }, 3500);
}

function setAuthMode(mode) {
  authMode = mode;
  pendingChallengeId = null;
  const isLogin = mode === "login";
  const isRegister = mode === "register";
  const tabs = [
    ["login", document.querySelector("#login-tab")],
    ["register", document.querySelector("#register-tab")]
  ];
  for (const [tabMode, tab] of tabs) {
    const active = tabMode === mode;
    tab.classList.toggle("is-active", active);
    tab.setAttribute("aria-selected", String(active));
  }
  authTitle.textContent = isLogin ? "Войти в аккаунт" : "Создать аккаунт";
  usernameLabel.hidden = isLogin;
  usernameInput.hidden = isLogin;
  usernameInput.required = isRegister;
  emailInput.required = true;
  credentialsFields.hidden = false;
  verificationFields.hidden = true;
  verificationCodeInput.required = false;
  authSubmit.textContent = "Получить код";
  setFormError(authError);
}

function showCodeEntry(challengeId, message) {
  pendingChallengeId = challengeId;
  credentialsFields.hidden = true;
  verificationFields.hidden = false;
  verificationMessage.textContent = message;
  verificationCodeInput.value = "";
  verificationCodeInput.required = true;
  authSubmit.textContent = "Подтвердить код";
  verificationCodeInput.focus();
}

function openLobby(lobby) {
  window.location.assign(`/lobby/${encodeURIComponent(lobby.id)}`);
}

async function leaveLobby(lobby) {
  try {
    if (lobby.isOwner) {
      await api(`/api/lobbies/${lobby.id}`, { method: "DELETE" });
    } else {
      await api(`/api/lobbies/${lobby.id}/leave`, { method: "POST", body: "{}" });
    }
    await loadLobbies();
    showStatus(lobby.isOwner ? "Лобби закрыто." : "Вы покинули лобби.");
  } catch (error) {
    showStatus(error.message);
  }
}

async function joinLobby(lobby) {
  try {
    await api(`/api/lobbies/${lobby.id}/join`, { method: "POST", body: "{}" });
    openLobby(lobby);
  } catch (error) {
    showStatus(error.message);
    if (error.status === 401) showAuth();
  }
}

async function startLobby(lobby) {
  try {
    await api(`/api/lobbies/${lobby.id}/start`, { method: "POST", body: "{}" });
    openLobby(lobby);
  } catch (error) {
    showStatus(error.message);
    if (error.status === 401) showAuth();
    else await loadLobbies();
  }
}

function addRoomCard(lobby) {
  const card = document.createElement("article");
  card.className = "room-card";

  const top = document.createElement("div");
  top.className = "room-card-top";
  const title = document.createElement("h3");
  title.textContent = lobby.name;
  const roomCode = document.createElement("span");
  roomCode.className = "room-code";
  roomCode.textContent = `КОМНАТА ${lobby.id}`;
  top.append(title, roomCode);

  const meta = document.createElement("p");
  meta.className = "room-meta";
  const members = lobby.players.map((player) => player.username).join(", ");
  meta.textContent = `Владелец: ${lobby.owner} · Игроки: ${members}`;

  const bottom = document.createElement("div");
  bottom.className = "room-card-bottom";
  const size = document.createElement("span");
  size.className = "room-size";
  size.textContent = `${lobby.players.length} из ${lobby.maxPlayers} игроков`;

  const actions = document.createElement("div");
  actions.className = "room-actions";
  const full = lobby.players.length >= lobby.maxPlayers;
  if (lobby.isMember) {
    const state = document.createElement("span");
    state.className = "room-size";
    state.textContent = lobby.isOwner ? "Вы владелец" : "Вы в комнате";
    actions.append(state);
    if (lobby.gameStatus === "waiting") {
      const openButton = document.createElement("button");
      openButton.className = "button button-quiet";
      openButton.type = "button";
      openButton.textContent = "Открыть комнату";
      openButton.addEventListener("click", () => openLobby(lobby));
      actions.append(openButton);
      if (lobby.isOwner && lobby.players.length >= 2) {
        const startButton = document.createElement("button");
        startButton.className = "button button-primary";
        startButton.type = "button";
        startButton.textContent = "Начать игру";
        startButton.addEventListener("click", () => startLobby(lobby));
        actions.append(startButton);
      } else {
        const leaveButton = document.createElement("button");
        leaveButton.className = "button button-quiet";
        leaveButton.type = "button";
        leaveButton.textContent = lobby.isOwner ? "Закрыть" : "Покинуть";
        leaveButton.addEventListener("click", () => leaveLobby(lobby));
        actions.append(leaveButton);
      }
    } else {
      const openButton = document.createElement("button");
      openButton.className = "button button-primary";
      openButton.type = "button";
      openButton.textContent = lobby.gameStatus === "finished" ? "Посмотреть партию" : "Войти в игру";
      openButton.addEventListener("click", () => openLobby(lobby));
      actions.append(openButton);
    }
  } else {
    const joinButton = document.createElement("button");
    joinButton.className = "button button-primary";
    joinButton.type = "button";
    joinButton.disabled = full || lobby.gameStatus !== "waiting";
    joinButton.textContent = lobby.gameStatus !== "waiting" ? "Игра началась" : full ? "Мест нет" : "Присоединиться";
    joinButton.addEventListener("click", () => joinLobby(lobby));
    actions.append(joinButton);
  }
  bottom.append(size, actions);
  card.append(top, meta, bottom);
  return card;
}

async function loadLobbies() {
  const { lobbies } = await api("/api/lobbies");
  roomsList.replaceChildren(...lobbies.map(addRoomCard));
  lobbyCount.textContent = String(lobbies.length);
  roomsEmpty.hidden = lobbies.length !== 0;
}

function showAuth() {
  activeUser = null;
  clearInterval(refreshTimer);
  authView.hidden = false;
  lobbyView.hidden = true;
  accountBar.hidden = true;
}

async function showLobbies(user) {
  await loadLobbies();
  activeUser = user;
  currentUser.textContent = user.username;
  accountBar.hidden = false;
  authView.hidden = true;
  lobbyView.hidden = false;
  clearInterval(refreshTimer);
  refreshTimer = setInterval(async () => {
    if (!activeUser) return;
    try {
      await loadLobbies();
    } catch (error) {
      if (error.status === 401) showAuth();
      else showStatus(error.message);
    }
  }, 5000);
}

document.querySelector("#login-tab").addEventListener("click", () => setAuthMode("login"));
document.querySelector("#register-tab").addEventListener("click", () => setAuthMode("register"));
document.querySelector("#back-to-credentials").addEventListener("click", () => {
  pendingChallengeId = null;
  verificationFields.hidden = true;
  credentialsFields.hidden = false;
  verificationCodeInput.required = false;
  authSubmit.textContent = "Получить код";
  setFormError(authError);
});

setAuthMode("login");

authForm.addEventListener("submit", async (event) => {
  event.preventDefault();
  setFormError(authError);
  const formData = new FormData(authForm);
  authSubmit.disabled = true;
  try {
    if (pendingChallengeId) {
      const { user } = await api("/api/auth/verify", {
        method: "POST",
        body: JSON.stringify({
          challengeId: pendingChallengeId,
          code: formData.get("code")
        })
      });
      authForm.reset();
      pendingChallengeId = null;
      await showLobbies(user);
    } else {
      const endpoint = authMode === "login" ? "/api/auth/login" : "/api/auth/register";
      const body = { email: formData.get("email") };
      if (authMode === "register") body.username = formData.get("username");
      const { challengeId, message } = await api(endpoint, {
        method: "POST",
        body: JSON.stringify(body)
      });
      showCodeEntry(challengeId, message);
    }
  } catch (error) {
    setFormError(authError, error.message);
  } finally {
    authSubmit.disabled = false;
  }
});

createForm.addEventListener("submit", async (event) => {
  event.preventDefault();
  setFormError(createError);
  const formData = new FormData(createForm);
  const submitButton = createForm.querySelector('button[type="submit"]');
  submitButton.disabled = true;
  try {
    const { lobby } = await api("/api/lobbies", {
      method: "POST",
      body: JSON.stringify({
        name: formData.get("name"),
        maxPlayers: Number(formData.get("maxPlayers"))
      })
    });
    createForm.reset();
    await loadLobbies();
    showStatus(`Лобби «${lobby.name}» создано. Код комнаты: ${lobby.id}`);
  } catch (error) {
    setFormError(createError, error.message);
  } finally {
    submitButton.disabled = false;
  }
});

document.querySelector("#refresh-button").addEventListener("click", async () => {
  try {
    await loadLobbies();
  } catch (error) {
    showStatus(error.message);
  }
});

document.querySelector("#logout-button").addEventListener("click", async () => {
  try {
    await api("/api/logout", { method: "POST", body: "{}" });
    showAuth();
    showStatus("Вы вышли из аккаунта.");
  } catch (error) {
    showStatus(error.message);
  }
});

async function initializeApp() {
  try {
    const { user } = await api("/api/me");
    await showLobbies(user);
  } catch (error) {
    if (error.status === 401) {
      showAuth();
    } else {
      showStatus(error.message);
    }
  }
}

initializeApp().catch((error) => {
  showStatus(error.message);
});
