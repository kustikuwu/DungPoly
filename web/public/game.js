const TOKEN_ASSETS = {
  1: "DOG.png",
  2: "HAT.png",
  3: "BOOT.png",
  4: "CAT.png",
  5: "CAR.png",
  6: "SHIP.png"
};
const GROUP_COLORS = [
  "#693232", "#24c2b5", "#d50070", "#ff8700",
  "#b60000", "#afa50f", "#1eaf0f", "#0f6daf"
];
const TILE_GLYPHS = {
  flag: '<path d="M5 21V4m1 1h11l-2 4 2 4H6"/> <path d="M8 7h5l-1 2 1 2H8" fill="#accent" stroke="#accent"/>',
  school: '<path d="m3 10 9-6 9 6v10H3z"/> <path d="M8 20v-6h8v6M7 10h.01M12 9h.01M17 10h.01"/> <path d="m7 4 5-2 5 2-5 3z" fill="#accent" stroke="#accent"/>',
  coin: '<circle cx="12" cy="12" r="9"/> <path d="M15 8.5c-.7-.8-1.6-1.2-3-1.2-1.5 0-2.5.8-2.5 2s1 1.8 2.5 2 2.5.8 2.5 2-1 2.2-2.8 2.2c-1.3 0-2.2-.4-3-1.2M12 5.5v13"/> <circle cx="12" cy="12" r="2" fill="#accent" stroke="#accent"/>',
  kebab: '<path d="M12 3v18"/> <path d="M7 6c2-2 8-2 10 0l-1 3c-2 1-6 1-8 0zM7 11c2-2 8-2 10 0l-1 3c-2 1-6 1-8 0zM7 16c2-2 8-2 10 0l-1 3c-2 1-6 1-8 0z" fill="#accent"/>',
  crown: '<path d="m3 8 5 4 4-7 4 7 5-4-2 11H5z"/> <circle cx="3" cy="7" r="1" fill="#accent" stroke="#accent"/> <circle cx="12" cy="4" r="1" fill="#accent" stroke="#accent"/> <circle cx="21" cy="7" r="1" fill="#accent" stroke="#accent"/> <path d="M6 16h12" stroke="#accent"/>',
  train: '<path d="M7 3h10a3 3 0 0 1 3 3v9a3 3 0 0 1-3 3H7a3 3 0 0 1-3-3V6a3 3 0 0 1 3-3Z"/> <path d="M5 12h14M8 18l-2 3m10-3 2 3"/> <path d="M7 6h4v3H7zm6 0h4v3h-4z" fill="#accent"/> <circle cx="8" cy="15" r="1" fill="#accent"/> <circle cx="16" cy="15" r="1" fill="#accent"/>',
  burger: '<path d="M4 10a8 7 0 0 1 16 0H4Zm0 3h16l-2 3H6zm2 7h12l-2-3H8z"/> <path d="M5 12h14v2H5z" fill="#accent"/> <circle cx="9" cy="7" r=".7" fill="#accent" stroke="#accent"/> <circle cx="14" cy="6" r=".7" fill="#accent" stroke="#accent"/>',
  gavel: '<path d="m5 7 6-4 4 6-6 4zM10 12l8 8m-3 1 6-6"/> <path d="m4 8 4 5m5-9 4 5" stroke="#accent"/> <path d="M3 21h7" stroke-width="2"/>',
  fries: '<path d="M5 9h14l-2 12H7z"/> <path d="m7 9 1-6m4 6V2m4 7-1-6"/> <path d="M7 13h10v3H7z" fill="#accent"/>',
  pizza: '<path d="m3 6 18-3-8 18z"/> <path d="m3 6 18-3"/> <circle cx="10" cy="10" r="1.2" fill="#accent"/> <circle cx="14" cy="8" r="1.2" fill="#accent"/> <circle cx="12" cy="15" r="1.2" fill="#accent"/> <path d="m5 7 8 14" stroke="#accent"/>',
  jail: '<path d="M4 4h16v16H4z"/> <path d="M8 4v16m4-16v16m4-16v16"/> <path d="M3 8h18M3 16h18" stroke="#accent"/>',
  ghost: '<path d="M5 20V10a7 7 0 1 1 14 0v10l-3-2-2 2-2-2-2 2-2-2z"/> <circle cx="9" cy="10" r="1" fill="#accent"/> <circle cx="15" cy="10" r="1" fill="#accent"/> <path d="M10 14h4" stroke="#accent"/>',
  energy: '<path d="m13 2-9 12h7l-1 8 10-13h-7z" fill="#accent"/>',
  blocks: '<path d="M4 4h7v7H4zm9 0h7v7h-7zM4 13h7v7H4zm9 0h7v7h-7z"/> <path d="M4 4h7v3H4zm9 9h7v3h-7z" fill="#accent"/>',
  sword: '<path d="m14 4 6-1-1 6-10 10-5-5z"/> <path d="m8 16-4 4m3-7 3 3m4-11 5 5" stroke="#accent"/> <path d="m17 4 3 5"/>',
  city: '<path d="M3 21V9l6-4v16m0-10h6V3l6 3v15z"/> <path d="M6 12h.01M6 16h.01m6-4h.01m0 4h.01m6-7h.01m0 4h.01m0 4h.01"/> <path d="M12 3h3v4h-3z" fill="#accent"/>',
  video: '<rect x="3" y="5" width="13" height="14" rx="2"/> <path d="m16 10 5-3v10l-5-3z"/> <path d="m8 9 4 3-4 3z" fill="#accent" stroke="#accent"/>',
  medal: '<circle cx="12" cy="9" r="6"/> <path d="m8 14-2 8 6-3 6 3-2-8"/> <path d="m12 6 1 2h2l-2 1.5.7 2.5-1.7-1.5-1.7 1.5.7-2.5L9 8h2z" fill="#accent" stroke="#accent"/>',
  gamepad: '<path d="M7 8h10a5 5 0 0 1 4.7 6.7l-1.1 3a2 2 0 0 1-3.2.8L15 16H9l-2.4 2.5a2 2 0 0 1-3.2-.8l-1.1-3A5 5 0 0 1 7 8Z"/> <path d="M7 11v4m-2-2h4" stroke="#accent"/> <circle cx="16" cy="12" r="1" fill="#accent"/> <circle cx="18" cy="15" r="1" fill="#accent"/>',
  moon: '<path d="M20 15.5A8 8 0 0 1 8.5 4 8 8 0 1 0 20 15.5Z"/> <path d="m15 4 .8 1.7 1.7.3-1.3 1.2.3 1.8L15 8l-1.5 1 .3-1.8-1.3-1.2 1.8-.3z" fill="#accent" stroke="#accent"/>',
  chat: '<path d="M4 4h16v12H9l-5 4z"/> <path d="M8 9h8m-8 4h5" stroke="#accent"/> <circle cx="7" cy="7" r=".7" fill="#accent" stroke="#accent"/>',
  house: '<path d="m3 11 9-8 9 8v10H5V11"/> <path d="M10 21v-6h4v6"/> <path d="M8 11h2m4 0h2" stroke="#accent"/>',
  factory: '<path d="M3 21V9l6 4V8l6 5V6l6 4v11z"/> <path d="M6 17h2m3 0h2m3 0h2"/> <path d="M16 6V3h3v6" stroke="#accent"/>',
  headphones: '<path d="M3 13v-2a9 9 0 0 1 18 0v2"/> <path d="M3 13h4v7H5a2 2 0 0 1-2-2zm14 0h4v5a2 2 0 0 1-2 2h-2z"/> <path d="M9 4a8 8 0 0 1 6 0" stroke="#accent"/>',
  plane: '<path d="m2 12 8 2 4 7 2-1-1-6 6 1 1-2-7-4-2-7-2 1 1 7-8 1z" fill="#accent"/>',
  bank: '<path d="m3 9 9-6 9 6zM5 10v8m5-8v8m8-8v8M3 21h18m-16-3h14"/> <path d="M12 4 6 8h12z" fill="#accent"/>',
  book: '<path d="M4 4h7a3 3 0 0 1 3 3v13a3 3 0 0 0-3-3H4z"/> <path d="M20 4h-3a3 3 0 0 0-3 3v13a3 3 0 0 1 3-3h3z"/> <path d="M6 8h3m8 0h1" stroke="#accent"/>',
  hazard: '<path d="m12 3 10 18H2z"/> <path d="M12 9v5m0 3h.01"/> <path d="m12 5 7 13H5z" stroke="#accent"/>',
  star: '<path d="m12 2 3 7 7 .5-5.5 4.5 2 8-6.5-4.5L5.5 22l2-8L2 9.5 9 9z" fill="#accent"/>',
  microphone: '<rect x="9" y="2" width="6" height="12" rx="3"/> <path d="M5 10v2a7 7 0 0 0 14 0v-2m-7 9v3m-4 0h8"/> <path d="M12 5v5" stroke="#accent"/>',
  gem: '<path d="m3 8 4-5h10l4 5-9 13z"/> <path d="M3 8h18M7 3l2 5m8-5-2 5m-4 0v13"/> <path d="m7 8 5 13 5-13z" fill="#accent"/>',
  suitcase: '<rect x="3" y="7" width="18" height="14" rx="2"/> <path d="M8 7V4h8v3m-6 0v14m4-14v14"/> <path d="M3 12h18" stroke="#accent"/>'
};
const TILE_ART = [
  "flag", "school", "coin", "kebab", "crown", "train", "burger", "gavel", "fries", "pizza",
  "jail", "ghost", "energy", "blocks", "sword", "city", "video", "medal", "gamepad", "blocks",
  "moon", "chat", "gavel", "coin", "microphone", "train", "hazard", "factory", "energy", "headphones",
  "jail", "city", "star", "coin", "house", "plane", "gavel", "gem", "suitcase", "bank"
];
const ACTIONS = [
  ["ROLL_DICE", "Бросить кубики", "ROLL_DICE.png", "ХОД"],
  ["END_TURN", "Завершить ход", "END_TURN.png", "ХОД"],
  ["BUY_FIELD", "Купить поле", "BUY_FIELD.png", "НЕДВИЖИМОСТЬ"],
  ["SELL_FIELD", "Продать поле", "SELL_FIELD_RU.png", "НЕДВИЖИМОСТЬ"],
  ["BUY_HOUSE", "Построить дом", "BUY_HOUSE.png", "НЕДВИЖИМОСТЬ"],
  ["SELL_HOUSE", "Продать дом", "SELL_HOUSE.png", "НЕДВИЖИМОСТЬ"],
  ["BUY_HOTEL", "Построить гостиницу", "BUY_HOTEL_RU.png", "НЕДВИЖИМОСТЬ"],
  ["SELL_HOTEL", "Продать гостиницу", "SELL_HOTEL_RU.png", "НЕДВИЖИМОСТЬ"],
  ["MORTGAGE_HOLDINGS", "Заложить владение", "MORTGAGE_HOLDINGS.png", "НЕДВИЖИМОСТЬ"],
  ["BUY_BACK_PROPERTY", "Выкупить из залога", "BUY_BACK_PROPERTY.png", "НЕДВИЖИМОСТЬ"],
  ["PAY_TAX", "Заплатить банку", "PAY_TAX_RU.png", "ПЛАТЕЖИ"],
  ["PAY_TO_OTHER_PLAYER", "Заплатить игроку", "PAY_TO_OTHER_PLAYER.png", "ПЛАТЕЖИ"],
  ["START_TRADE", "Предложить обмен", "START_TRADE.png", "ОБМЕН И КАРТЫ"],
  ["START_TRADE_NEW_FIELD", "Начать аукцион", "START_TRADE_NEW_FIELD.png", "ОБМЕН И КАРТЫ"],
  ["TAKE_CARD", "Взять карточку", "TAKE_CARD.png", "ОБМЕН И КАРТЫ"],
  ["USE_CARD", "Использовать карточку", "USE_CARD.png", "ОБМЕН И КАРТЫ"],
  ["EXIT_GAME", "Сдаться", "EXIT_GAME.png", "ИГРА"]
].map(([id, label, icon, group]) => ({ id, label, icon, group }));
const ACTION_BY_ID = new Map(ACTIONS.map((action) => [action.id, action]));
const ROOM_ID = window.location.pathname.match(/^\/lobby\/([A-F0-9]{8})$/)?.[1];

const connectionStatus = document.querySelector("#connection-status");
const roomError = document.querySelector("#room-error");
const actionList = document.querySelector("#action-list");
const propertyActions = document.querySelector("#property-actions");
const playersList = document.querySelector("#players-list");
const sumForm = document.querySelector("#sum-form");
const sumInput = document.querySelector("#sum-value");
const tradeControls = document.querySelector("#trade-controls");
let socket;
let lobby;
let gameState;
let currentRequest;
let selectedTile = null;
let pendingTile;
let lastEvent = "Подключение к игровой комнате";
let currentUsername = "";
let replyPending = false;

function showError(message) {
  roomError.textContent = message;
  roomError.hidden = false;
}

function setConnectionStatus(message, state = "normal") {
  connectionStatus.textContent = message;
  connectionStatus.dataset.state = state;
}

async function api(path, options = {}) {
  const response = await fetch(path, {
    ...options,
    headers: {
      ...(options.body ? { "Content-Type": "application/json" } : {}),
      ...options.headers
    },
    credentials: "same-origin"
  });
  const result = await response.json().catch(() => ({}));
  if (!response.ok) throw new Error(result.error ?? `Ошибка сервера (${response.status}).`);
  return result;
}

function assetUrl(name) {
  return `/resources/${encodeURIComponent(name)}`;
}

function renderLobby() {
  if (!lobby) return;
  document.querySelector("#room-name").textContent = lobby.name;
  document.querySelector("#room-code").textContent = `КОД КОМНАТЫ: ${lobby.id}`;
  document.querySelector("#current-user").textContent = currentUsername;
  const startButton = document.querySelector("#start-game");
  const canStart = ["waiting", "interrupted", "failed"].includes(lobby.gameStatus);
  startButton.hidden = !lobby.isOwner || !canStart;
  startButton.disabled = lobby.players.length < 2;
  startButton.textContent = lobby.gameStatus === "waiting" ? "Начать игру" : "Перезапустить партию";
  if (lobby.gameStatus === "playing") {
    setConnectionStatus("Соединение с игровым сервером установлено");
  } else if (lobby.gameStatus === "finished") {
    setConnectionStatus("Партия завершена");
  } else if (lobby.gameStatus === "waiting") {
    setConnectionStatus(`Ожидание начала партии · ${lobby.players.length} из ${lobby.maxPlayers} игроков`);
  } else if (lobby.gameStatus === "interrupted") {
    setConnectionStatus("Партия прервана перезапуском сервера. Владелец может начать заново.", "error");
  } else if (lobby.gameStatus === "failed") {
    setConnectionStatus("Партия завершилась с ошибкой. Владелец может попробовать запустить её снова.", "error");
  }
  renderPlayers();
}

function tileEdge(position) {
  if (position < 10) return "edge-left";
  if (position < 20) return "edge-top";
  if (position < 30) return "edge-right";
  return "edge-bottom";
}

function playerBadge(token, className = "") {
  const badge = document.createElement("span");
  badge.className = `player-badge token-color-${token} ${className}`;
  return badge;
}

function tileIllustration(tile) {
  const glyph = TILE_GLYPHS[TILE_ART[tile.position]];
  if (!glyph) throw new Error(`Для клетки ${tile.position} не задана иллюстрация.`);
  const accent = GROUP_COLORS[tile.color] ?? "#ffd77d";
  const svg = `<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24"><g fill="none" stroke="#f1f3f7" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round">${glyph.replaceAll("#accent", accent)}</g></svg>`;
  const image = document.createElement("img");
  image.className = "tile-illustration";
  image.src = `data:image/svg+xml,${encodeURIComponent(svg)}`;
  image.alt = "";
  image.setAttribute("aria-hidden", "true");
  return image;
}

function renderBoard() {
  const board = document.querySelector("#game-board");
  const dashboard = document.querySelector("#center-dashboard");
  board.replaceChildren();
  if (!gameState) {
    board.append(dashboard);
    return;
  }
  const tiles = gameState.tiles ?? [];
  for (const tile of tiles) {
    const button = document.createElement("button");
    button.type = "button";
    button.className = `board-tile ${tileEdge(tile.position)}${[0, 10, 20, 30].includes(tile.position) ? " board-corner" : ""}`;
    button.classList.add(`tile-pos-${tile.position}`);
    button.dataset.position = String(tile.position);
    button.setAttribute("aria-label", `Клетка ${tile.position + 1}: ${tile.name}`);
    button.title = tile.name;
    button.setAttribute("aria-pressed", String(selectedTile === tile.position));

    if (tile.color !== null && tile.color !== undefined && GROUP_COLORS[tile.color]) {
      const colorBand = document.createElement("span");
      colorBand.className = `tile-color group-color-${tile.color}`;
      button.append(colorBand);
    }

    if (tile.owner !== null && tile.owner !== 0) {
      const ownerMarker = document.createElement("span");
      ownerMarker.className = `tile-owner token-color-${tile.owner}`;
      button.append(ownerMarker);
    }

    button.append(tileIllustration(tile));

    if (tile.cost !== null && tile.cost !== undefined) {
      const cost = document.createElement("span");
      cost.className = "tile-cost";
      cost.textContent = `${tile.cost} М`;
      button.append(cost);
    }
    if (tile.mortgaged) {
      const mortgage = document.createElement("span");
      mortgage.className = "tile-mortgage";
      mortgage.textContent = "З";
      button.append(mortgage);
    }

    const markers = document.createElement("span");
    markers.className = "tile-markers";
    for (const player of (gameState.players ?? []).filter((entry) => entry.alive && entry.position === tile.position)) {
      const marker = playerBadge(player.token, "token-marker");
      marker.title = player.name;
      markers.append(marker);
    }
    button.append(markers);
    button.addEventListener("click", () => selectTile(tile.position));
    board.append(button);
  }

  board.append(dashboard);
  renderPropertyCard();
}

function currentPlayer() {
  return gameState?.players?.find((player) => player.token === gameState.currentPlayer && player.alive);
}

function renderPlayers() {
  playersList.replaceChildren();
  const players = gameState?.players ?? lobby?.players ?? [];
  for (const player of players) {
    const canChoose = currentRequest?.type === "TOKEN" && player.alive !== false;
    const card = document.createElement(canChoose ? "button" : "div");
    card.className = "player-card";
    if (player.token === gameState?.currentPlayer) card.classList.add("current");
    if (canChoose) {
      card.type = "button";
      card.disabled = player.token === currentRequest.playerToken;
      card.title = `Выбрать игрока ${player.name}`;
      card.addEventListener("click", () => sendReply("TOKEN", String(player.token)));
    }

    if (player.token && TOKEN_ASSETS[player.token]) card.append(playerBadge(player.token));
    const info = document.createElement("span");
    info.className = "player-info";
    const name = document.createElement("span");
    name.className = "player-name";
    name.textContent = player.name ?? player.username;
    info.append(name);
    if (player.money !== undefined) {
      const propertyCount = gameState.tiles.filter((tile) => tile.owner === player.token).length;
      const money = document.createElement("span");
      money.className = "player-money";
      money.textContent = `${player.money} М · владений: ${propertyCount}${player.prisoner ? " · тюрьма" : ""}${player.alive === false ? " · банкрот" : ""}`;
      info.append(money);
    }
    card.append(info);
    playersList.append(card);
  }
}

function selectTile(position) {
  if (currentRequest?.type === "NUM") {
    selectedTile = position;
    for (const tile of document.querySelectorAll(".board-tile")) {
      tile.setAttribute("aria-pressed", String(Number(tile.dataset.position) === position));
    }
    renderPropertyCard();
    pendingTile = undefined;
    sendReply("NUM", String(position));
    return;
  }
  selectedTile = position;
  for (const tile of document.querySelectorAll(".board-tile")) {
    tile.setAttribute("aria-pressed", String(Number(tile.dataset.position) === position));
  }
  renderPropertyCard();
}

function formatTileDetails(tile) {
  const owner = tile.owner && tile.owner !== 0
    ? gameState.players.find((player) => player.token === tile.owner)?.name ?? "неизвестен"
    : null;
  const lines = [
    `Статус: ${owner ? "В собственности" : "Свободно"}`,
    ...(tile.rent !== null && tile.rent !== undefined ? [`Базовая аренда: ${tile.rent} М`] : []),
    ...(owner ? [`Владелец: ${owner}`] : []),
    `Залог: ${tile.mortgaged ? "Да" : "Нет"}`,
    ...(tile.houses !== null && tile.houses !== undefined
      ? [`Дома: ${tile.houses}   Гостиницы: ${tile.hotels ?? 0}`]
      : []),
    ...(tile.houseCost !== null && tile.houseCost !== undefined ? [`Стоимость дома: ${tile.houseCost} М`] : []),
    ...(tile.hotelCost !== null && tile.hotelCost !== undefined ? [`Стоимость гостиницы: ${tile.hotelCost} М`] : []),
    ...(tile.taxes ? [`Аренда с постройками: ${tile.taxes.join(" / ")} М`] : [])
  ];
  return lines.join("\n");
}

function renderPropertyCard() {
  if (!gameState?.tiles?.length) return;
  const position = selectedTile ?? currentPlayer()?.position ?? 0;
  const tile = gameState.tiles[position] ?? gameState.tiles[0];
  const propertyColor = document.querySelector("#property-color");
  propertyColor.className = "property-color";
  if (tile.color !== null && tile.color !== undefined && GROUP_COLORS[tile.color]) {
    propertyColor.classList.add(`group-color-${tile.color}`);
  }
  document.querySelector("#property-name").textContent = tile.name;
  const price = document.querySelector("#property-price");
  price.hidden = tile.cost === null || tile.cost === undefined;
  price.textContent = price.hidden ? "" : `ЦЕНА  ${tile.cost} М`;
  document.querySelector("#property-details").textContent = formatTileDetails(tile);
  renderPropertyActions(tile);
}

function canUseSelectedTile() {
  return currentRequest?.type === "ACTION";
}

function renderPropertyActions(tile) {
  propertyActions.replaceChildren();
  const actionsHint = document.querySelector("#property-actions-hint");
  actionsHint.hidden = true;
  if (!tile || !gameState) {
    document.querySelector("#property-actions-title").hidden = true;
    return;
  }
  const isOwnable = tile.cost !== null && tile.cost !== undefined;
  if (!isOwnable) {
    document.querySelector("#property-actions-title").hidden = true;
    return;
  }
  const selectedPlayer = currentPlayer();
  const isOwner = tile.owner === selectedPlayer?.token;
  const isLanding = (tile.owner === 0 || tile.owner === null || tile.owner === undefined) &&
    selectedPlayer?.position === tile.position;
  const applicable = new Set([
    ...(isLanding ? ["BUY_FIELD"] : []),
    ...(isOwner && tile.color !== null && !tile.mortgaged && (tile.houses ?? 0) < 4 && !(tile.hotels ?? 0)
      ? ["BUY_HOUSE"] : []),
    ...(isOwner && tile.color !== null && !tile.mortgaged && (tile.houses ?? 0) >= 4 && !(tile.hotels ?? 0)
      ? ["BUY_HOTEL"] : []),
    ...(isOwner && (tile.houses ?? 0) > 0 && !(tile.hotels ?? 0) ? ["SELL_HOUSE"] : []),
    ...(isOwner && (tile.hotels ?? 0) > 0 ? ["SELL_HOTEL"] : []),
    ...(isOwner && !(tile.houses ?? 0) && !(tile.hotels ?? 0) && !tile.mortgaged
      ? ["SELL_FIELD", "MORTGAGE_HOLDINGS"] : []),
    ...(isOwner && tile.mortgaged ? ["BUY_BACK_PROPERTY"] : [])
  ]);
  const actionsTitle = document.querySelector("#property-actions-title");
  actionsTitle.hidden = applicable.size === 0;
  const hasAvailableAction = currentRequest?.type === "ACTION" &&
    [...applicable].some((action) => currentRequest.actions.includes(action));
  actionsHint.hidden = applicable.size === 0 || hasAvailableAction;
  for (const action of ACTIONS.filter((entry) => applicable.has(entry.id))) {
    const button = document.createElement("button");
    button.type = "button";
    button.className = "property-action";
    button.disabled = replyPending || !canUseSelectedTile() ||
      !currentRequest?.actions.includes(action.id);
    const icon = document.createElement("img");
    icon.src = assetUrl(action.icon);
    icon.alt = "";
    button.append(icon, document.createTextNode(action.label));
    button.addEventListener("click", () => {
      if (action.id !== "BUY_FIELD") pendingTile = tile.position;
      sendReply("ACTION", action.id);
    });
    propertyActions.append(button);
  }
}

function renderActionPanel() {
  actionList.replaceChildren();
  const hint = document.querySelector("#turn-input-hint");
  hint.textContent = "";
  sumForm.hidden = true;
  tradeControls.hidden = true;
  const request = currentRequest;
  const ownTurn = Boolean(request && request.playerToken === gameState?.currentPlayer);
  document.querySelector("#action-status").textContent = request
    ? replyPending ? "Обрабатываем ответ..." :
      ownTurn ? request.message || "Выберите действие" : request.message || "Выполняется действие"
    : gameState ? "Ожидание хода" : "Ожидание партии";

  if (request?.type === "ACTION") {
    let activeGroup;
    for (const action of ACTIONS.filter((entry) => request.actions.includes(entry.id))) {
      if (action.group !== activeGroup) {
        activeGroup = action.group;
        const groupTitle = document.createElement("h3");
        groupTitle.className = "action-group";
        groupTitle.textContent = activeGroup;
        actionList.append(groupTitle);
      }
      const button = document.createElement("button");
      button.type = "button";
      button.className = "action-button";
      if (action.id === "ROLL_DICE") button.classList.add("action-button-primary");
      button.title = action.label;
      button.setAttribute("aria-label", action.label);
      button.disabled = replyPending;
      const image = document.createElement("img");
      image.src = assetUrl(action.icon);
      image.alt = "";
      const label = document.createElement("span");
      label.textContent = action.label;
      button.append(image, label);
      button.addEventListener("click", () => sendReply("ACTION", action.id));
      actionList.append(button);
    }
  } else if (request?.type === "NUM") {
    hint.textContent = request.message || "Выберите клетку на поле.";
  } else if (request?.type === "SUM") {
    hint.textContent = request.message || "Введите сумму и подтвердите.";
    sumForm.hidden = false;
    sumInput.disabled = replyPending;
    sumForm.querySelector("button[type='submit']").disabled = replyPending;
    sumInput.focus();
  } else if (request?.type === "TOKEN") {
    hint.textContent = request.message || "Выберите игрока в центре поля.";
  } else if (request?.type === "TRADE") {
    hint.textContent = request.message || "Участвовать в обмене?";
    tradeControls.hidden = false;
    tradeControls.querySelectorAll("button").forEach((button) => { button.disabled = replyPending; });
  } else if (gameState) {
    hint.textContent = "Сейчас ход другого игрока.";
  }

  const propertyPosition = selectedTile ?? currentPlayer()?.position ?? 0;
  renderPropertyActions(gameState?.tiles?.[propertyPosition]);
  renderPlayers();
}

function renderGameState() {
  if (!gameState) return;
  const player = currentPlayer();
  const ownTurn = currentRequest?.playerToken === gameState.currentPlayer;
  const banner = document.querySelector("#turn-banner");
  if (player) {
    banner.classList.remove(...Object.keys(TOKEN_ASSETS).map((token) => `token-outline-${token}`));
    banner.classList.add(`token-outline-${player.token}`);
    const turnToken = document.querySelector("#turn-token");
    turnToken.className = `token-badge token-color-${player.token}`;
    document.querySelector("#turn-label").textContent = `ХОД ИГРОКА: ${player.name}`;
    document.querySelector("#active-player").textContent = player.name;
    const activeBadge = document.querySelector("#active-badge");
    activeBadge.className = `player-badge token-color-${player.token}`;
    activeBadge.hidden = false;
    const propertyCount = gameState.tiles.filter((tile) => tile.owner === player.token).length;
    document.querySelector("#active-summary").textContent =
      `Баланс: ${player.money} М   •   Владений: ${propertyCount}`;
    let turnStatus = !player.alive ? "Банкрот"
      : player.prisoner ? "В Доске Позора"
        : currentRequest?.type === "SUM" ? "Введите сумму платежа"
          : currentRequest?.type === "NUM" ? "Выберите клетку на поле"
            : currentRequest?.type === "TOKEN" ? "Выберите игрока"
              : currentRequest?.type === "TRADE" ? "Ожидает решения по сделке"
                : currentRequest?.actions?.includes("ROLL_DICE") ? "Ваш ход — бросьте кубики"
                  : currentRequest?.actions?.includes("END_TURN") ? "Завершите ход, когда будете готовы"
                    : "Ожидание следующего действия";
    turnStatus += `   •   ${gameState.tiles[Math.min(player.position, 39)]?.name ?? ""}`;
    document.querySelector("#active-status").textContent = turnStatus;
  }
  if (currentRequest?.type === "NUM" && pendingTile !== undefined) {
    const position = pendingTile;
    pendingTile = undefined;
    sendReply("NUM", String(position));
  }
  renderBoard();
  renderActionPanel();
  if (ownTurn) setConnectionStatus("Ваш ход");
}

function sendReply(replyType, value) {
  if (!currentRequest || replyPending || !socket || socket.readyState !== WebSocket.OPEN) {
    showError("Нет активного игрового запроса. Подождите обновления состояния.");
    return;
  }
  socket.send(JSON.stringify({
    type: "reply",
    requestId: currentRequest.id,
    replyType,
    value: String(value)
  }));
  replyPending = true;
  renderActionPanel();
}

function diceText(message) {
  const match = message.match(/^\s*(\d+)\s+(\d+)\s*$/);
  if (!match) return;
  const pipsByValue = {
    1: [4],
    2: [0, 8],
    3: [0, 4, 8],
    4: [0, 2, 6, 8],
    5: [0, 2, 4, 6, 8],
    6: [0, 2, 3, 5, 6, 8]
  };
  const firstDie = document.querySelector("#die-one");
  const secondDie = document.querySelector("#die-two");
  const setFace = (die, value) => {
    die.replaceChildren(...Array.from({ length: 9 }, (_, index) => {
      const pip = document.createElement("span");
      pip.className = `die-pip${pipsByValue[value]?.includes(index) ? " is-visible" : ""}`;
      return pip;
    }));
  };
  const finalFaces = [Number(match[1]), Number(match[2])];
  const dice = [firstDie, secondDie];
  const title = document.querySelector("#dice-title");
  const result = document.querySelector("#dice-result");
  const notice = document.querySelector("#dice-notice");
  title.textContent = "Бросаем кубики...";
  result.textContent = "";
  notice.classList.add("dice-rolling");
  notice.hidden = false;
  window.clearTimeout(diceText.timer);
  window.clearTimeout(diceText.hideTimer);
  window.clearInterval(diceText.rollInterval);
  const startedAt = Date.now();
  const updateRollingFaces = () => {
    const frame = Math.floor((Date.now() - startedAt) / 65);
    dice.forEach((die, index) => setFace(die, (frame * (index + 2) + index * 3) % 6 + 1));
  };
  updateRollingFaces();
  diceText.rollInterval = window.setInterval(updateRollingFaces, 65);
  diceText.timer = window.setTimeout(() => {
    window.clearInterval(diceText.rollInterval);
    notice.classList.remove("dice-rolling");
    dice.forEach((die, index) => setFace(die, finalFaces[index]));
    title.textContent = "Результат броска";
    result.textContent = `Выпало: ${match[1]} + ${match[2]}`;
    diceText.hideTimer = window.setTimeout(() => { notice.hidden = true; }, 400);
  }, 1100);
}

function handleGameEvent(event) {
  if (event.type === "lobby") {
    lobby = event.lobby;
    renderLobby();
  } else if (event.type === "state") {
    gameState = event.state;
    renderGameState();
  } else if (event.type === "request") {
    currentRequest = event.request;
    replyPending = Boolean(event.replyPending);
    if (currentRequest.type === "NUM" && pendingTile !== undefined) {
      const position = pendingTile;
      pendingTile = undefined;
      sendReply("NUM", String(position));
      return;
    }
    renderActionPanel();
    if (currentRequest.type === "ACTION") setConnectionStatus("Ваш ход");
  } else if (event.type === "waiting") {
    currentRequest = null;
    replyPending = false;
    document.querySelector("#action-status").textContent = `Ход игрока ${event.player}`;
    document.querySelector("#turn-label").textContent = `ХОД ИГРОКА: ${event.player}`;
    setConnectionStatus(`Ход игрока ${event.player}`);
    renderActionPanel();
  } else if (event.type === "notification") {
    if (event.messageType === 3) {
      diceText(event.message);
    } else {
      lastEvent = event.message;
      document.querySelector("#last-event").textContent = event.message;
    }
  } else if (event.type === "game_started") {
    if (lobby) {
      lobby.gameStatus = "playing";
      renderLobby();
    }
  } else if (event.type === "game_over") {
    gameState = event.state ?? gameState;
    currentRequest = null;
    replyPending = false;
    if (lobby) lobby.gameStatus = "finished";
    document.querySelector("#last-event").textContent = "Партия завершена";
    renderLobby();
    renderGameState();
  } else if (event.type === "reply_error") {
    currentRequest = event.request;
    replyPending = false;
    showError(event.message);
    renderActionPanel();
  } else if (event.type === "game_error") {
    currentRequest = null;
    replyPending = false;
    setConnectionStatus("Ошибка игровой сессии", "error");
    showError(event.message);
    renderActionPanel();
  } else if (event.type === "error") {
    showError(event.message);
  }
}

async function startGame() {
  try {
    const result = await api(`/api/lobbies/${ROOM_ID}/start`, {
      method: "POST",
      body: JSON.stringify({})
    });
    lobby = result.lobby;
    renderLobby();
  } catch (error) {
    showError(error.message);
  }
}

async function initialize() {
  if (!ROOM_ID) {
    window.location.replace("/");
    return;
  }
  try {
    const { user } = await api("/api/me");
    currentUsername = user.username;
    document.querySelector("#current-user").textContent = user.username;
    renderBoard();
    const protocol = window.location.protocol === "https:" ? "wss:" : "ws:";
    socket = new WebSocket(`${protocol}//${window.location.host}/api/game?lobby=${encodeURIComponent(ROOM_ID)}`);
    socket.addEventListener("open", () => setConnectionStatus("Подключено к игровой комнате"));
    socket.addEventListener("message", (event) => {
      try {
        handleGameEvent(JSON.parse(event.data));
      } catch (error) {
        showError(`Ошибка игрового сообщения: ${error.message}`);
      }
    });
    socket.addEventListener("close", () => setConnectionStatus("Соединение с комнатой закрыто", "error"));
    socket.addEventListener("error", () => setConnectionStatus("Не удалось подключиться к комнате", "error"));
  } catch (error) {
    if (error.message.includes("Войдите")) {
      window.location.replace("/");
      return;
    }
    setConnectionStatus("Не удалось загрузить комнату", "error");
    showError(error.message);
  }
}

document.querySelector("#start-game").addEventListener("click", startGame);
sumForm.addEventListener("submit", (event) => {
  event.preventDefault();
  const value = Number(sumInput.value);
  if (!Number.isSafeInteger(value) || value < 0 || value > 1000000000) {
    showError("Введите целое число от 0 до 1 000 000 000.");
    return;
  }
  sendReply("SUM", String(value));
  sumInput.value = "";
});
document.querySelector("#trade-accept").addEventListener("click", () => sendReply("TRADE", "PARTICIPATE"));
document.querySelector("#trade-refuse").addEventListener("click", () => sendReply("TRADE", "REFUSE"));
document.querySelector("#logout-button").addEventListener("click", async () => {
  try {
    await api("/api/logout", { method: "POST", body: JSON.stringify({}) });
  } catch (error) {
    showError(error.message);
    return;
  }
  socket?.close();
  window.location.replace("/");
});

document.querySelector("#last-event").textContent = lastEvent;
initialize();
