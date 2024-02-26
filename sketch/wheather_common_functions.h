#ifndef COMMON_FUNCTIONS_H_
#define COMMON_FUNCTIONS_H_
float mm_to_inches(float value_mm);
float hPa_to_inHg(float value_hPa);
float SumOfPrecip(float DataArray[], int readings);

#endif /* ifndef COMMON_FUNCTIONS_H_ */

float mm_to_inches(float value_mm)
{
  return 0.0393701 * value_mm;
}

float hPa_to_inHg(float value_hPa)
{
  return 0.02953 * value_hPa;
}

float SumOfPrecip(float DataArray[], int readings) {
  float sum = 0;
  for (int i = 0; i < readings; i++) {
    sum += DataArray[i];
  }
  return sum;
}
