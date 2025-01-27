import { getCalendarData } from "../utils/calendar";
import Month from "./Month"
import PropTypes from 'prop-types';
import './Page.css'

function Page({monthOfYear, currentYear}) {
  const prevMonth = getCalendarData(monthOfYear === 1 ? 12 : monthOfYear -1, monthOfYear === 1 ? currentYear - 1 : currentYear);
  const nextMonth = getCalendarData(monthOfYear === 12 ? 1 : monthOfYear + 1, monthOfYear === 12 ? currentYear + 1 : currentYear);
  const currentMonth = getCalendarData(monthOfYear, currentYear); 
  return (
    <div className="page-container">
      <div className="page-header">
        <div className="prev-month">
          <Month calendarData={prevMonth.cal} month={prevMonth.month} year={prevMonth.year} size="small" />
        </div>
        <div className="current-month-name">
          <h1>{currentMonth.month} {currentMonth.year}</h1>
        </div>
        <div className="next-month">
          <Month calendarData={nextMonth.cal} month={nextMonth.month} year={nextMonth.year} size="small" />
        </div>
      </div>
      <div className="current-month">
        <Month calendarData={currentMonth.cal} size="big" />
      </div>      
    </div>
      
  )
}
Page.propTypes = {
  monthOfYear: PropTypes.number.isRequired,
  currentYear: PropTypes.number.isRequired,
};
export default Page