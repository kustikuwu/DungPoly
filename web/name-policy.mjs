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
const POLICY_TERMS = [
  "hate speech", "разжигание ненависти", "racism", "расизм",
  "racial slur", "расовое оскорбление", "racial discrimination", "расовая дискриминация",
  "ethnic discrimination", "этническая дискриминация", "sexism", "сексизм",
  "misogyny", "женоненавистничество", "misandry", "мужененавистничество",
  "homophobia", "гомофобия", "transphobia", "трансфобия", "biphobia", "бифобия",
  "xenophobia", "ксенофобия", "caste discrimination", "дискриминация по касте",
  "ableism", "дискриминация по инвалидности", "hate symbol", "hate group", "hateful tag",
  "hateful username", "racist username", "racist tag", "sexual tag", "pornographic tag",
  "nudity-related tag", "drug-related tag", "self-harm tag", "suicide-related tag",
  "harassment tag", "threatening tag", "vulgar tag", "glorification of tragedies",
  "нацист", "nazi", "white supremacy", "превосходство белой расы",
  "dehumanization", "дегуманизация",   "genocide denial", "genocide", "геноцид", "отрицание геноцида",
  "conversion therapy", "конверсионная терапия", "harassment", "травля", "травл",
  "домогательств", "оскорб", "дискримин",
  "bullying", "буллинг", "cyberbullying", "targeted harassment", "personal attack",
  "угроза убийством", "death threat", "threat", "угроз", "physical violence",
  "violence", "физическое насилие",
  "encouraging violence", "призыв к насилию", "wishing death", "пожелание смерти",
  "encouraging suicide", "призыв к самоубийству", "self-harm", "самоповреждение",
  "stalking", "сталкинг", "doxxing", "доксинг", "brigading", "hate raid",
  "impersonation", "выдача себя за другого", "deepfake", "дипфейк",
  "pornography", "порнография", "sexual content", "сексуальный контент",
  "sexual act", "sexual roleplay", "мастурбация", "sexual intercourse",
  "oral sex", "anal sex", "sexual fetish", "erotic content", "эротический контент",
  "sexual service", "prostitution", "проституция", "escort service", "sexual solicitation",
  "sexual violence", "сексуальное насилие", "сексуаль", "насил", "rape threat", "угроза изнасилованием",
  "sexual exploitation", "сексуальная эксплуатация", "revenge porn",
  "sexual harassment", "сексуальные домогательства", "nudity", "нагота",
  "exposed genitals", "sexualized body", "csam", "child safety", "child sexual abuse",
  "child sexual exploitation", "child grooming", "груминг несовершеннолетних",
  "sexualization of minors", "сексуализация несовершеннолетних",
  "minor nudity", "нагота несовершеннолетних", "solicitation of minors",
  "склонение несовершеннолетних", "child trafficking", "торговля детьми",
  "graphic violence", "графическое насилие", "extreme violence", "extreme gore", "расчленение",
  "torture", "пытки", "execution footage", "записи казней", "murder",
  "убийство", "убийств", "terrorism", "терроризм", "terrorist propaganda",
  "террористическая пропаганда", "violent extremism", "насильственный экстремизм",
  "terrorist recruitment", "вербовка в террористические организации",
  "bomb threat", "угроза взрывом", "school shooting", "стрельба в школах",
  "suicide", "самоубийство", "self-mutilation", "анорексия", "anorexia",
  "bulimia", "булимия", "drug trafficking", "наркоторговля",
  "illegal activities", "незаконная деятельность", "illegal weapon sales",
  "illegal drug", "запрещённые наркотики", "наркот", "drug abuse", "substance abuse",
  "drug manufacturing",
  "производство наркотиков", "firearms trafficking", "торговля оружием",
  "human trafficking", "торговля людьми", "forced labor", "принудительный труд",
  "robbery", "ограбление", "vandalism", "вандализм", "money laundering",
  "отмывание денег", "counterfeit goods", "поддельные товары", "illegal gambling",
  "незаконные азартные игры", "criminal instructions", "криминальные инструкции",
  "dangerous activities", "опасные действия", "dangerous stunt", "опасный трюк",
  "reckless driving", "безрассудное вождение",
  "drunk driving", "вождение в нетрезвом состоянии", "dangerous weapon",
  "опасное обращение с оружием", "dangerous explosives", "взрывчатые вещества",
  "spam", "спам", "message flooding", "массовая отправка сообщений",
  "unauthorized advertising", "несанкционированная реклама", "phishing", "фишинг",
  "scamming", "мошенничество", "account theft", "кража аккаунтов",
  "malware", "вредоносное по", "virus distribution", "распространение вирусов",
  "malicious link", "вредоносная ссылка", "fake giveaway", "фальшивый розыгрыш",
  "fraud", "theft", "краж", "fraudulent",
  "viewbotting", "накрутка просмотров", "account selling", "продажа аккаунтов",
  "privacy violation", "нарушение конфиденциальности", "intellectual property",
  "интеллектуальная собственность", "other prohibited behavior", "запрещённое поведение",
  "personal information leak", "утечка личной информации", "private address",
  "личный адрес", "phone number leak", "утечка номера телефона",
  "email address leak", "утечка адреса электронной почты", "ip address leak",
  "утечка ip адреса", "private messages", "личная переписка", "private videos",
  "личные видео", "private photos", "личные фотографии", "unauthorized surveillance",
  "несанкционированная слежка", "leaking financial information",
  "утечка финансовых данных", "copyright infringement", "нарушение авторских прав",
  "pirated content", "пиратский контент", "illegal gambling promotion",
  "продвижение нелицензированных азартных игр", "offensive username",
  "оскорбительный никнейм", "threatening username", "угрожающий никнейм",
  "drug-related username", "никнейм с наркотиками", "ban evasion",
  "обход блокировки", "circumventing safety systems", "обход систем безопасности",
  "hacking twitch services", "взлом сервисов twitch", "exploiting platform vulnerabilities",
  "использование уязвимостей платформы", "evading moderation", "обход модерации", "platform manipulation",
  "манипуляция платформой", "service disruption", "нарушение работы сервиса"
];

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
const NORMALIZED_POLICY_TERMS = POLICY_TERMS.map(canonicalize).filter(Boolean);
const POLICY_NAME_PARTS = new Set(NORMALIZED_POLICY_TERMS);

export function containsBannedName(value) {
  const canonicalName = canonicalize(value);
  const parts = value.split(/[^\p{L}\p{N}]+/u).map(canonicalize).filter(Boolean);
  if (parts.some((part) => BANNED_WORDS.has(part) || POLICY_NAME_PARTS.has(part))) return true;

  return [...BANNED_WORDS].some((word) => word.length >= 4 && canonicalName.includes(word)) ||
    NORMALIZED_POLICY_TERMS.some((term) => term.length >= 4 && canonicalName.includes(term));
}
