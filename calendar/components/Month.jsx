import './Month.css'
import PropTypes from 'prop-types';
import classNames from 'classnames'
import { weekDays, weekDaysLong, isHoliday } from '../utils/calendar'; 
function Month({calendarData, monthOfYear, month, year, size}) {
  const days = size === 'big' ? weekDaysLong : weekDays;
  return (
    <>
      {size !== 'big' && <div className="month-name">{month} {year}</div>}
      <div className={classNames('month-container', { 'big': size === 'big', 'small': size !== 'big' })}>
        <div className="month-header days-of-week">
          {days.map((day, index) => (<div key={index}>{day}</div>))}
        </div>
        {calendarData.map((week, weekIndex) => (
          <div key={weekIndex} className="month-week">
            {week.map((day, dayIndex) => (
              <div key={dayIndex} className={classNames('month-day', { 
                'holiday': isHoliday(day), 
                'current': day.getMonth() === monthOfYear - 1 
                })}>
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
  monthOfYear: PropTypes.number.isRequired,
  size: PropTypes.string.isRequired
};

export default Month
