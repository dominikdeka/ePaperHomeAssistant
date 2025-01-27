import './Month.css'
import PropTypes from 'prop-types';
import { weekDays, weekDaysLong } from '../utils/calendar'; 
function Month({calendarData, month, year, size}) {
  const days = size === 'big' ? weekDaysLong : weekDays;
  const classNames = size === 'big' ? 'month-container big' : 'month-container small';
  return (
    <>
      {size !== 'big' && <div className="month-name">{month} {year}</div>}
      <div className={classNames}>
        <div className="month-header days-of-week">
          {days.map((day, index) => (<div key={index}>{day}</div>))}
        </div>
        {calendarData.map((week, weekIndex) => (
          <div key={weekIndex} className="month-week">
            {week.map((day, dayIndex) => (
              <div key={dayIndex} className="month-day">
                {day.getDate()}
              </div>
            ))}
          </div>
        ))}
      </div>
    </>
  )
}
Month.propTypes = {
  calendarData: PropTypes.object.isRequired,
  year: PropTypes.string.isRequired,
  month: PropTypes.string.isRequired,
  size: PropTypes.string.isRequired
};

export default Month
