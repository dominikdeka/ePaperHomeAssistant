function doGet() {
  const events = GetEvents();
  Logger.log(JSON.stringify(events, null, 2));
  return ContentService.createTextOutput(JSON.stringify(events)).setMimeType(ContentService.MimeType.JSON); 
}
const DAYS_NUMBER = 31;
const letters     = ['ą', 'ć', 'ę', 'ł', 'ń', 'ó', 'ś', 'ź', 'ż', 'Ą', 'Ć', 'Ę', 'Ł', 'Ń', 'Ó', 'Ś', 'Ź', 'Ż'];
const replacement = ['a', 'c', 'e', 'l', 'n', 'o', 's', 'z', 'z', 'A', 'C', 'E', 'L', 'N', 'O', 'S', 'Z', 'Z'];
function replacePolishLetters(text) {
  let result = text;

  for (let i = 0; i < letters.length; i++) {
      result = result.replaceAll(letters[i], replacement[i]);
  }
  return result;
}

function formatDateToTime(date) {
  var hours = date.getHours();
  var minutes = date.getMinutes();
  minutes = minutes < 10 ? '0'+minutes : minutes;
  return hours + ':' + minutes;
}
const weekDaysPL = ['nd', 'pn', 'wt', 'sr', 'czw', 'pt', 'so' ]
function formatDateToDay(date) {
  let mm = date.getMonth() + 1; // Months start at 0!
  let dd = date.getDate();
  const weekDay = weekDaysPL[date.getDay()];

  if (dd < 10) dd = '0' + dd;
  if (mm < 10) mm = '0' + mm;

  return `${dd}/${mm} ${weekDay}`;
}

function formatDuration(startDate, endDate, isAllDay) {
 if(isAllDay){
  return Math.ceil(Math.abs(endDate - startDate) / (1000*60*60*24)) + 'd'
 } else {
  const hours = Math.floor(Math.abs(endDate - startDate) / (1000*60*60));
  const minutes = Math.floor(Math.abs(endDate - startDate) / (1000*60)) % 60;
  return (hours > 0 ? `${hours}h` : '') + ((hours>0 && minutes>0) ? ' ' : '') + (minutes > 0 ? `${minutes}m` : '')
 }
}

function GetEvents() {
  var date = new Date();
    var Now = new Date();
    var Later = new Date();
    Later.setDate(Now.getDate() + DAYS_NUMBER);

  var firstDay = new Date(date.getFullYear(), date.getMonth(), 1);
  var lastDay = new Date(date.getFullYear(), date.getMonth() + 1, 0);
  var calendars = CalendarApp.getAllCalendars();
  let events = [];
  for (const calendar of calendars) {
    events = events.concat(calendar.getEvents(Now, Later));
    // Logger.log(calendar.getName());
  }
  
  // Logger.log(Later);
  // Logger.log(events.length);
  let results = [];
  for (let i = 0; i < events.length; i++){
    const event = events[i];
    const isAllDay = event.isAllDayEvent();
    const startDateTime = event.getStartTime();
    const endDateTime = event.getEndTime();
    const duration = formatDuration(startDateTime, endDateTime, event.isAllDayEvent());

    let name = '';
    if (!isAllDay) {
      name = `${formatDateToTime(startDateTime)} `;
    }
    let title = replacePolishLetters(event.getTitle());
    name += (title > 30 
      ? title.slice(0, 27) + '...' 
      : title);
    name += (duration !== '1d' ? ` (${duration})` : '');
    const eventObj = {
      start: formatDateToDay(startDateTime),
      startDateTime: startDateTime,
      name: name 
    }
    results.push(eventObj);
  }
  results = results
    .sort(function(a,b){
      return a.startDateTime - b.startDateTime
    })
    .slice(0, 30)
    .map((event) => { delete event.startDateTime; return event; });

  results = results.reduce((acc, d) => {
    const found = acc.find(a => a.start === d.start);
    //const value = { name: d.name, val: d.value };
    const value = d.name; // the element in data property
    if (!found) {
      //acc.push(...value);
      acc.push({start:d.start, events: [d.name]}) // not found, so need to add data property
    }
    else {
      //acc.push({ name: d.name, data: [{ value: d.value }, { count: d.count }] });
      found.events.push(d.name) // if found, that means data property exists, so just push new element to found.data.
    }
    return acc;
  }, []);
  return results;
}