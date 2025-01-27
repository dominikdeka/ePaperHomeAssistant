const Calendar = {
  currentDate: new Date(),
  getCurrentMonthDays: function() {
    const year = this.currentDate.getFullYear();
    const month = this.currentDate.getMonth();
    const firstDay = new Date(year, month, 1);
    const lastDay = new Date(year, month + 1, 0);
    const days = [];

    for (let day = firstDay.getDate(); day <= lastDay.getDate(); day++) {
      days.push(new Date(year, month, day));
    }

    return days;
  },
  render: function() {
    const days = this.getCurrentMonthDays();
    days.forEach(day => {
      console.log(day.toDateString());
    });
  }
};

// Example usage:
Calendar.render();