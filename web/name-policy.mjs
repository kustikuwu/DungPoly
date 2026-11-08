import leoProfanity from "leo-profanity";

const CONFUSABLES = new Map([
  ["а", "a"], ["е", "e"], ["ё", "e"], ["о", "o"], ["р", "p"],
  ["с", "c"], ["х", "x"], ["у", "y"], ["к", "k"], ["м", "m"],
  ["т", "t"], ["в", "b"], ["н", "h"], ["і", "i"], ["ї", "i"],
  ["ј", "j"], ["ѕ", "s"], ["ԁ", "d"], ["զ", "q"]
]);
const LEETSPEAK = new Map([
  ["0", "o"], ["1", "i"], ["3", "e"], ["4", "a"], ["5", "s"],
  ["7", "t"], ["8", "b"], ["9", "g"], ["@", "a"], ["$", "s"]
]);

function canonicalize(value) {
  return [...value.normalize("NFKD").toLowerCase()]
    .filter((character) => !/\p{M}/u.test(character))
    .map((character) => CONFUSABLES.get(character) ?? LEETSPEAK.get(character) ?? character)
    .join("")
    .replace(/[^\p{L}\p{N}]/gu, "");
}

function loadWords(language) {
  leoProfanity.loadDictionary(language);
  return leoProfanity.list().map(canonicalize).filter(Boolean);
}

const BANNED_WORDS = new Set([...loadWords("en"), ...loadWords("ru")]);

export function containsBannedName(value) {
  const canonicalName = canonicalize(value);
  const parts = value.split(/[^\p{L}\p{N}]+/u).map(canonicalize).filter(Boolean);
  if (parts.some((part) => BANNED_WORDS.has(part))) return true;

  return [...BANNED_WORDS].some((word) => word.length >= 4 && canonicalName.includes(word));
}
