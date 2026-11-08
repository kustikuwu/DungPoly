function backendUrl(env, requestUrl) {
  if (typeof env?.DUNGPOLY_BACKEND_ORIGIN !== "string") {
    return new Response("Backend origin is not configured.", { status: 503 });
  }

  let origin;
  try {
    origin = new URL(env.DUNGPOLY_BACKEND_ORIGIN);
  } catch {
    return new Response("Backend origin is invalid.", { status: 503 });
  }

  const isLocalHttp = origin.protocol === "http:" &&
    ["localhost", "127.0.0.1", "[::1]"].includes(origin.hostname);
  if ((origin.protocol !== "https:" && !isLocalHttp) ||
      origin.username || origin.password || origin.pathname !== "/" ||
      origin.search || origin.hash) {
    return new Response("Backend origin must be an HTTPS origin.", { status: 503 });
  }

  const incoming = new URL(requestUrl);
  return { destination: new URL(`${incoming.pathname}${incoming.search}`, origin) };
}

export async function onRequest({ request, env }) {
  const result = backendUrl(env, request.url);
  if (result instanceof Response) return result;
  return fetch(new Request(result.destination, request));
}
