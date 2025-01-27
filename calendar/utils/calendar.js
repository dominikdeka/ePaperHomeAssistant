import { 
  isSameDay,
  startOfMonth, 
  endOfMonth, 
  eachDayOfInterval, 
  getWeek, 
  getDay,
  addDays } from 'date-fns';
const MONTHS = [
  'Styczeń',
  'Luty',
  'Marzec',
  'Kwiecień',
  'Maj',
  'Czerwiec',
  'Lipiec',
  'Sierpień',
  'Wrzesień',
  'Październik',
  'Listopad',
  'Grudzień',
];
const WEEKDAYS_SHORT = ['Pon', 'Wt', 'Śr', 'Czw', 'Pt', 'Sob', 'Nd'];
const WEEKDAYS_LONG = [
  'Poniedziałek',
  'Wtorek',
  'Środa',
  'Czwartek',
  'Piątek',
  'Sobota',
  'Niedziela',
];

function getCalendarData(monthOfYear, year) {
  let start = startOfMonth(new Date(year, monthOfYear - 1));
  start = addDays(start, -(getDay(start) === 0 ? 6 : getDay(start) - 1));
  let end = endOfMonth(new Date(year, monthOfYear - 1));
  end = addDays(end, getDay(end) === 0 ? 0 : 7 - getDay(end));
  const days = eachDayOfInterval({ start, end });

  const cal = [];
  let prevWeek = '';
  let index = 0;
  days.forEach(day => {
    const week = getWeek(day, { weekStartsOn: 1 });
    if(week !== prevWeek) {
      index++;
      prevWeek = week;
    }
    const dayOfWeek = getDay(day) === 0 ? 6 : getDay(day) - 1;
    if (!cal[index]) {
      cal[index] = [];
    }
    cal[index][dayOfWeek] = day;
  });

  return { 
    cal, 
    month: MONTHS[monthOfYear - 1],
    year 
  }; 
}
const HOLIDAYS = [
  { month: 0, day: 1 }, // Nowy Rok
  { month: 0, day: 6 }, // Trzech Króli
  { month: 4, day: 1 }, // Święto Pracy
  { month: 4, day: 3 }, // Święto Konstytucji 3 Maja
  { month: 7, day: 15 }, // Wniebowzięcie NMP
  { month: 10, day: 1 }, // Wszystkich Świętych
  { month: 10, day: 11 }, // Święto Niepodległości
  { month: 11, day: 25 }, // Boże Narodzenie
  { month: 11, day: 26 }, // Drugi dzień Bożego Narodzenia
];

function calculateEaster(year) {
  const f = Math.floor,
    G = year % 19,
    C = f(year / 100),
    H = (C - f(C / 4) - f((8 * C + 13) / 25) + 19 * G + 15) % 30,
    I = H - f(H / 28) * (1 - f(H / 28) * f(29 / (H + 1)) * f((21 - G) / 11)),
    J = (year + f(year / 4) + I + 2 - C + f(C / 4)) % 7,
    L = I - J,
    month = 3 + f((L + 40) / 44),
    day = L + 28 - 31 * f(month / 4);
  return new Date(year, month - 1, day);
}
function calculateEasterMonday(year) {
  return addDays(calculateEaster(year), 1);
}
function calculateCorpusChristi(year) {
  return addDays(calculateEaster(year), 60);
}

function isHoliday(day) {
  console.log(day);
  return HOLIDAYS.some(holiday => holiday.month === (day.getMonth()) && holiday.day === day.getDate())
    || isSameDay(day, calculateEaster(day.getFullYear()))
    || isSameDay(day, calculateEasterMonday(day.getFullYear()))
    || isSameDay(day, calculateCorpusChristi(day.getFullYear()))
    || isWeekend(day);
}

function isWeekend(day) {
  return getDay(day) === 0 || getDay(day) === 6;
}

export {
  isHoliday,
  getCalendarData,
  WEEKDAYS_SHORT as weekDays,
  WEEKDAYS_LONG as weekDaysLong
};