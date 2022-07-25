/*  
	Source: https://www.subsystems.us/arduino-project---eclipses.html

    This program calculates the date of eclipses of the moon for a given year.
    The program will calculate the eclipses for that year. It will give the date 
    and the times of the penumbral, umbral, and totalality of the eclipse.
    
    Calculation taken from Sky & Telescope June, 1988.

    Visit subsystems.us for more fun Arduino and science projets.
*/

void MoonEclipse( int tYear, // input year
                  int pDate[], // output packed time: 100*month + day
                  int EclipseYear[] // this routine gives a date in 2028 when tYear = 2027, 2029
                  ) 
  {
  float Ya = tYear;
  float U = 0;
  float Rad1 = 3.14159265 / 180;
  float K0 = (long)((Ya - 1900) * 12.3685);
  float Ta = (Ya - 1899.5) / 100;
  float T2 = Ta * Ta;
  float T3 = Ta * Ta * Ta;
  float J0 = 2415020 + 29 * K0;
  float F0 = 0.0001178 * T2 - 0.000000155 * T3;
  F0 = F0 + 0.75933 + 0.53058868 * K0;
  F0 = F0 - 0.000837 * Ta - 0.000335 * T2;
  J0 = J0 + (long)(F0);
  F0 = F0 - (long)(F0);
  float M0 = K0 * 0.08084821133;
  M0 = 360.0 * (M0 - (int)(M0)) + 359.2242;
  M0 = M0 - 0.0000333 * T2;
  M0 = M0 - 0.00000347 * T3;
  float M1 = K0 * 0.07171366128;
  M1 = 360 * (M1 - (long)(M1)) + 306.0253;
  M1 = M1 + 0.0107306 * T2;
  M1 = M1 + 0.00001236 * T3;
  float B1a = K0 * 0.08519585128;
  B1a = 360 * (B1a - (long)(B1a)) + 21.2964;
  B1a = B1a - 0.0016528 * T2;
  B1a = B1a - 0.00000239 * T3;

  int dateCounter = 0;
  
  for (int K9 = 1; K9 < 28; K9 = K9 + 2) {
    float J = J0 + 14 * K9;
    float F = F0 + 0.765294 * K9;
    float K = (float)K9 / 2.0;
    float M5 = (M0 + K * 29.10535608) * Rad1;
    float M6 = (M1 + K * 385.81691806) * Rad1;
    float B6 = (B1a + K * 390.67050646) * Rad1;
    F = F - 0.4068 * sin(M6);
    F = F + (0.1734 - 0.000393 * Ta) * sin(M5);
    F = F + 0.0161 * sin(2 * M6);
    F = F - 0.0104 * sin(2 * B6);
    F = F - 0.0074 * sin(M5 - M6);
    F = F - 0.0051 * sin(M5 + M6);
    F = F + 0.0021 * sin(2 * M5);
    F = F + 0.5 / 1440;
    J = J + (long)(F);
    F = F - (long)(F);
    // calc eqlipse
    float D7 = 0;
    if (abs(sin(B6)) <= 0.36) {
      float S = 5.19595 - 0.0048 * cos(M5);
      S = S + 0.0020 * cos(2 * M5);
      S = S - 0.3283 * cos(M6);
      S = S - 0.0060 * cos(M5 + M6);
      S = S + 0.0041 * cos(M5 - M6);
      float C1 = 0.2070 * sin(M5);
      C1 = C1 + 0.0024 * sin(2 * M5);
      C1 = C1 - 0.0390 * sin(M6);
      C1 = C1 + 0.0115 * sin(2 * M6);
      C1 = C1 - 0.0073 * sin(M5 + M6);
      C1 = C1 - 0.0067 * sin(M5 - M6);
      C1 = C1 + 0.0117 * sin(2 * B6);
      float D9 = abs(S * sin(B6) + C1 * cos(B6));

      U = 0.0059 + 0.0046 * cos(M5);
      U = U - 0.0182 * cos(M6);
      U = U + 0.0004 * cos(2 * M6);
      U = U - 0.0005 * cos(M5 + M6);
      float RP = 1.2847 + U;
      float RU = 0.7404 - U;
      float MP = (1.5572 + U - D9) / 0.545;
      if (MP < 0) continue;
      float MU = (1.0129 - U - D9) / 0.545;
      float D5 = 1.5572 + U;
      float D6 = 1.0129 - U;
      D7 = 0.4679 - U;
      float N = (0.5458 + 0.04 * cos(M6)) / 60;
      D5 = sqrt(D5 * D5 - D9 * D9) / N;
      if (MU > 0) D6 = sqrt(D6 * D6 - D9 * D9) / N;
      if (MU > 1) D7 = sqrt(D7 * D7 - D9 * D9) / N;

      // convert Julian to M/D/Y
      F = F + 0.5;
      if (F >= 1) {
        F = F - 1;
        J = J + 1;
      }
      float A1 = (long)((J / 36524.25) - 51.12264);
      float Aa = J + 1 + A1 - (long)(A1 / 4);
      float Ba = Aa + 1524;
      float Ca = (long)((Ba / 365.25) - 0.3343);
      float Da = (long)(365.25 * Ca);
      float E = (long)((Ba - Da) / 30.61);
      Da = Ba - Da - (long)(30.61 * E) + F;
      float M = E - 1;
      float Ya = Ca - 4716;
      if ( E > 13.5) M = M - 12;
      if ( M < 2.5) Ya = Ya + 1;
      float H = 24 * (Da - (long)Da);
      float M9 = (int)(60 * (H - (int)H));

      pDate[dateCounter]        = 100*M + Da; // compact date
      EclipseYear[dateCounter]  = Ya; 
      dateCounter = dateCounter  + 1;

#ifdef FEATURE_SERIAL_LUNARECLIPSE

      // Print the results

      Serial.println();
      Serial.print(F("Eclipse date: "));
      Serial.print((int)M);
      Serial.print("/");
      Serial.print((int)Da);
      Serial.print("/");
      Serial.println((int)Ya);
      Serial.println();
      Serial.print(F("Maximum Phase: "));
      Serial.print((int)H);
      Serial.print("h ");
      Serial.print((int)M9);
      Serial.println("m UTC");
      MP = (int)(1000 * MP + 0.5);
      MP = MP / 1000.0;
      Serial.print(F("Penumbral Mag: "));
      Serial.println(MP);
      if (MU > 0) {
        MU = (int)(1000 * MU + 0.5);
        MU = MU / 1000.0;
        Serial.print(F("Umbral Mag: "));
        Serial.println(MU);
      }
      Serial.println("Semidurations: ");
      Serial.print("  Penumbra: ");
      Serial.print((int)(D5 + 0.5));
      Serial.println("m");
      if (MU >= 0) {
        Serial.print("  Umbra: ");
        Serial.print((int)(D6 + 0.5));
        Serial.println("m");
        Serial.print("  Totality: ");
        Serial.print((int)(D7 + 0.5));
        Serial.println("m");
      }
#endif
    }
  }
  
  pDate[dateCounter] = 0;
}
