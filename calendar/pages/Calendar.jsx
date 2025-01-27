import Page from "../components/Page"
import './Calendar.css'
function Calendar() {
  const currentYear = new Date().getFullYear();
  return (
    <>
      {Array.from({ length: 12 }, (_, index) => (
        <Page key={index} currentYear={currentYear} monthOfYear={index + 1} />
      ))}
    </>    
  )
}

export default Calendar
