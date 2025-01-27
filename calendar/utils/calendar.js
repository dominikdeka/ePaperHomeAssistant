import { 
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

export {
  getCalendarData,
  WEEKDAYS_SHORT as weekDays,
  WEEKDAYS_LONG as weekDaysLong
};