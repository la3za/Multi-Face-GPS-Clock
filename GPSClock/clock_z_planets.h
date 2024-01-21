//------------------------------------------------------------------------------------------------------------------
// From https://github.com/mobifu1/arduino_planet_ephi_positions/blob/master/Planet_ephi_positions.ino
// mobifu1 / arduino_planet_ephi_positions
//------------------------------------------------------------------------------------------------------------------
// Calculation the Positions of Planets by elliptic Orbit in the Solarsystem between Year 2000-2100
// http://denknix.com/astro/doc/html/section003.html
// http://ssd.jpl.nasa.gov/horizons.cgi
// http://www.jgiesen.de/kepler/kepler.html   Java applet: keppler equation
// Author: Andreas Jahnke, aajahnke@aol.com
//------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------

// Sechs Bahnelemente:                                                                 / Variable:
// a: Länge der großen Halbachse                                                       / [0] semi major axis in AE
// e: numerische Exzentrizität                                                         / [2] eccentricity
// i: Bahnneigung, Inklination                                                         / [4] inclination
// L                                                                                   / [6] meanLongitude
// omega: Argument der Periapsis, Periapsisabstand                                     / [8] longitude of perihelion
// Omega: Länge/Rektaszension des aufsteigenden Knotens                                / [10] longitude ascending node

// Tables:
//// String object_name[8] = {"Mercury", "Venus", "Earth", "Mars", "Jupiter", "Saturn", "Uranus", "Neptune"};
//String star_name[1] = {"Sun"};

// http://ssd.jpl.nasa.gov/txt/aprx_pos_planets.pdf
const float object_data[6][13] = {// a, aΔ, e, eΔ, i, iΔ,  L, LΔ, ω, ωΔ, Ω, ΩΔ  >>> L2000 , diameter
  {0.38709927, 0.00000037, 0.20563593, 0.00001906, 7.00497902, -0.00594749, 252.25032350, 149472.67411175, 77.45779628, 0.16047689, 48.33076593, -0.12534081, 6.74},   // Mercury
  {0.72333566, 0.00000390, 0.00677672, -0.00004107, 3.39467605, -0.00078890, 181.97909950, 58517.81538729, 131.60246718, 0.00268329, 76.67984255, -0.27769418, 16.92}, // Venus
  {1.00000261, 0.00000562, 0.01671123, -0.00004392, -0.00001531, -0.01294668, 100.46457166, 35999.37244981, 102.93768193, 0.32327364, 0, 0, 0},                        // Earth
  {1.52371034, 0.00001847, 0.09339410, 0.00007882, 1.84969142, -0.00813131, -4.55343205, 19140.30268499, -23.94362959, 0.44441088, 49.55953891, -0.29257343, 9.31},    // Mars
  {5.20288700, -0.00011607, 0.04838624, -0.00013253, 1.30439695, -0.00183714, 34.39644051, 3034.74612775, 14.72847983, 0.21252668, 100.47390909, 0.20469106, 191},     // Jupiter
  {9.53667594, -0.00125060, 0.05386179, -0.00050991, 2.48599187, 0.00193609, 49.95424423, 1222.49362201, 92.59887831, -0.41897216, 113.66242448, -0.28867794, 157},    // Saturn
//  {19.1891646, -0.00196176, 0.04725744, -0.00004397, 0.77263783, -0.00242939, 313.23810451, 428.48202785, 170.95427630, 0.40805281, 074.01692503, 0.04240589, 64},     // Uranus
//  {30.06992276, 0.00026291, 0.00859048, 0.00005105, 1.77004347, 0.00035372, -55.12002969, 218.45945325, 44.96476227, -0.32241464, 131.78422574, -0.00508664, 61.5},    // Neptun
};

// global factors:
const float rad = 0.017453293; // deg to rad
const float deg = 57.29577951; // rad to deg
const float pi = 3.1415926535; // PI
float jd;                      // Juliane date
float jd_frac;
float eclipticAngle = 23.43928;

//global coordinates:
float x_coord;
float y_coord;
float z_coord;

float x_earth;
float y_earth;
float z_earth;

float azimuthPlanet;
float altitudePlanet;

float phase;
float magnitude = 0;

//float lat = 53.5; //GPS Position of Hamburg in deg
//float lon = 10;

float lat = 59.83; //GPS Position of Asker, Norway in deg
float lonPlanet = 10.43;
boolean full = false; // full output of planeterary output on serial port, otherwise bare minimum (only if EATURE_SERIAL_PLANETARY is set)


float ra; //deg
float dec;
float dist_earth_to_object;
float dist_earth_to_sun;
float dist_object_to_sun;

//------------------------------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------------------------------
float get_julian_date (float day_, float month_, float year_, float hour_, float minute_, float seconds_) { // UTC

  if (month_ <= 2) {
    year_ -= 1;
    month_ += 12;
  }

  long A = year_ / 100;
  long B = A / 4;
  long C = 2 - A + B;
  long E = 365.25 * (year_ + 4716);
  long F = 30.6001 * (month_ + 1);
  jd = C + day_ + E + F - 1524.5;
  jd_frac = (hour_ / 24) + (minute_ / 1440) + (seconds_ / 86400);
  return jd;
}
//------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------
float calc_format_angle_deg (float deg) {  //0-360 degrees

  if (deg >= 360 || deg < 0) {
    if (deg < 0) {
      while (deg < 0) {
        deg += 360;
      }
    }
    long x = (long)deg;
    float comma = deg - x;
    long y = x % 360; //modulo 360
    return comma += y;
  }
  return deg;
}
//------------------------------------------------------------------------------------------------------------------
float calc_eccentricAnomaly (float meanAnomaly, float eccentricity) {

  meanAnomaly *= rad;

  int iterations = 0;
  float eccentricAnomaly = meanAnomaly + (eccentricity * sin(meanAnomaly));
  //Serial.println(String(eccentricAnomaly, DEC));
  float deltaEccentricAnomaly = 1;

  while (fabs(deltaEccentricAnomaly) > 0.000001) { // 0.0000001

    deltaEccentricAnomaly = (meanAnomaly - eccentricAnomaly + (eccentricity * sin(eccentricAnomaly))) / (1 - eccentricity * cos(eccentricAnomaly));
    //Serial.println(String(deltaEccentricAnomaly, DEC));
    eccentricAnomaly += deltaEccentricAnomaly;
    //Serial.println(String(eccentricAnomaly, DEC));

    iterations++;
    if (iterations > 20) {
      //Serial.println(F("Error:Keplergleichung!!!!!")); // this will not upload https://code.google.com/archive/p/arduino/issues/459
      #ifdef FEATURE_SERIAL_PLANETARY
        Serial.println(F("Error:Keplergleichung!"));
      #endif  
      eccentricAnomaly = 0;
      break;
    }
  }
  eccentricAnomaly *= deg;
  return eccentricAnomaly;
}



//------------------------------------------------------------------------------------------------------------------
void format_angle(float angle, String format) {

  int d = 0;
  int m = 0;
  int s = 0;
  float rest = 0;
  String sign = "";

  if (format == F("degrees") || format == F("degrees-latitude")) {

    rest = calc_format_angle_deg (angle);

    if (format == F("degrees-latitude") && rest > 90) {
      rest -= 360;
    }
    if (rest >= 0) {
      sign = "+";
    }
    else {
      sign = "-";
    }

    rest = fabs(rest);
    d = (int)(rest);
    rest = (rest - (float)d) * 60;
    m = (int)(rest);
    rest = (rest - (float)m) * 60;
    s = (int)rest;
    #ifdef FEATURE_SERIAL_PLANETARY
      if (full) Serial.println(sign + String(d) + ":" + String(m) + ":" + String(s));
    #endif
  }
}
//--------------------------------------------------------------------------------------------------------------------
void rot_x(float alpha) {

  alpha *= rad;
  float y = cos(alpha) * y_coord - sin(alpha) * z_coord;
  float z = sin(alpha) * y_coord + cos(alpha) * z_coord;
  y_coord = y;
  z_coord = z;
}
//------------------------------------------------------------------------------------------------------------------
void rot_y (float alpha) {

  alpha *= rad;
  float x = cos(alpha) * x_coord + sin(alpha) * z_coord;
  float z = sin(alpha) * x_coord + cos(alpha) * z_coord;
  x_coord = x;
  z_coord = z;
}
//------------------------------------------------------------------------------------------------------------------
void rot_z (float alpha) {

  alpha *= rad;
  float x = cos(alpha) * x_coord - sin(alpha) * y_coord;
  float y = sin(alpha) * x_coord + cos(alpha) * y_coord;
  x_coord = x;
  y_coord = y;
}
//------------------------------------------------------------------------------------------------------------------
void calc_vector_subtract(float xe, float xo, float ye, float yo, float ze, float zo) {

  x_coord = xo - xe;
  y_coord = yo - ye;
  z_coord = zo - ze;
}
//------------------------------------------------------------------------------------------------------------------
float calc_siderealTime (float jd, float jd_frac, float lon) { //03:50:00 = 2457761.375

  float T = jd - 2451545;
  T /= 36525;
  float UT = jd_frac * 24;
  float T0 = 6.697374558 + (2400.051336 * T) + (0.000025862 * T * T) + (UT * 1.0027379093);
  T0 = fmod(T0, 24);
  float siderial_time = T0 + (lon / 15);
  return siderial_time;
}
//------------------------------------------------------------------------------------------------------------------
void calc_azimuthal_position(float ra, float dec, float lat, float sidereal_time) {

  float ha = (sidereal_time * 15) - ra; //ha = hours of angle  (-180 to 180 deg)
  if (ha < -180) ha += 360;
  if (ha > 180) ha -= 360;
  if (dec < -90) dec += 360;
  if (dec > 90) dec -= 360;

  ha *= rad;
  dec *= rad;
  lat *= rad;

  float x = cos(ha) * cos(dec);
  float y = sin(ha) * cos(dec);
  float z = sin(dec);

  //rotate y
  float x_hor = x * sin(lat) - z * cos(lat);//horizon position
  float y_hor = y;
  float z_hor = x * cos(lat) + z * sin(lat);

  azimuthPlanet = atan2(y_hor, x_hor) + pi;
  altitudePlanet = atan2(z_hor, sqrt(x_hor * x_hor + y_hor * y_hor));
  azimuthPlanet *= deg;//0=north, 90=east, 180=south, 270=west
  altitudePlanet *= deg;//0=horizon, 90=zenith, -90=down
  #ifdef FEATURE_SERIAL_PLANETARY
    Serial.println("*** azimuth:" + String(azimuthPlanet, DEC));
    Serial.println("*** altitude:" + String(altitudePlanet, DEC));
    Serial.println("*** distance:" + String(dist_earth_to_object, DEC));
  #endif
  }
//------------------------------------------------------------------------------------------------------------------
void calc_magnitude(int object_number, float R) {// R = distance earth to object in AE

  float apparent_diameter = object_data[object_number][12] / R;
  #ifdef FEATURE_SERIAL_PLANETARY
    if (full) Serial.print("apparent diameter:" + String(apparent_diameter, 2));//bogen seconds
    if (object_number == 5 & full)Serial.print(" + ring = " + String(apparent_diameter + 20, 2)); //bogen seconds
    if (full) Serial.println();
  #endif
  
  float r = dist_object_to_sun;//r = distance in AE
  float s = dist_earth_to_sun;//s = distance in AE

  float elon = acos((s * s + R * R - r * r) / (2 * s * R));
  elon *= deg;
  #ifdef FEATURE_SERIAL_PLANETARY
    if (full) Serial.println("elongation:" + String(elon, 2));
  #endif

  float phase_angle = acos((r * r + R * R - s * s) / (2 * r * R));
  //float 
  phase = (1 + cos(phase_angle)) / 2;
  phase_angle *= deg;
  #ifdef FEATURE_SERIAL_PLANETARY
    if (full) Serial.println("phase angle:" + String(phase_angle, 2));
    Serial.println("phase:" + String(phase, 2));
  #endif

  //float magnitude = 0;
  float ring_magn = -0.74;
  if (object_number == 0) magnitude = -0.36 + 5 * log10(r * R) + 0.027 * phase_angle;             //Mercury
  if (object_number == 1) magnitude = -4.34 + 5 * log10(r * R) + 0.013 * phase_angle;             //Venus
  if (object_number == 3) magnitude = -1.51 + 5 * log10(r * R) + 0.016 * phase_angle;             //Mars
  if (object_number == 4) magnitude = -9.25 + 5 * log10(r * R) + 0.014 * phase_angle;             //Jupiter
  if (object_number == 5) magnitude = -9.00 + 5 * log10(r * R) + 0.044 * phase_angle + ring_magn; //Saturn
  if (object_number == 6) magnitude = -7.15 + 5 * log10(r * R) + 0.001 * phase_angle;             //Uranus
  if (object_number == 7) magnitude = -6.90 + 5 * log10(r * R) + 0.001 * phase_angle;             //Neptune
  #ifdef FEATURE_SERIAL_PLANETARY
    Serial.println("magnitude:" + String(magnitude, 2));
  #endif
}

//------------------------------------------------------------------------------------------------------------------
void calc_vector(float x, float y, float z, String mode) {

  // convert to rectangular coordinates:
  if (mode == F("to_rectangular")) {

    x *= rad;
    y *= rad;

    x_coord = z * cos(x) * cos(y);
    y_coord = z * cos(x) * sin(y);
    z_coord = z * sin(x);

    x = x_coord;
    y = y_coord;
    z = z_coord;
  }

  //  Serial.println("x_coord:" + String(x, DEC));
  //  Serial.println("y_coord:" + String(y, DEC));
  //  Serial.println("z_coord:" + String(z, DEC));

  // convert to spherical coordinates:
  //get Longitude:
  float lonPlanet = atan2(y, x);
  lonPlanet *= deg;
  lonPlanet = calc_format_angle_deg (lonPlanet);
  ra = lonPlanet;
  #ifdef FEATURE_SERIAL_PLANETARY
    if (full) Serial.println("LON:" + String(lonPlanet, DEC));
  #endif
  format_angle(lonPlanet, F("degrees"));

  //get Latitude:
  float lat = atan2(z, (sqrt(x * x + y * y)));
  lat *= deg;
  lat = calc_format_angle_deg (lat);
  dec = lat;
  #ifdef FEATURE_SERIAL_PLANETARY
    if (full) Serial.println("LAT:" + String(lat, DEC));
  #endif
  format_angle(lat, F("degrees-latitude"));

  //getDistance:
  dist_earth_to_object = sqrt(x * x + y * y + z * z);
  #ifdef FEATURE_SERIAL_PLANETARY
    if (full) Serial.println("DIS:" + String(dist_earth_to_object, DEC));
  #endif
}

//------------------------------------------------------------------------------------------------------------------
void calc_orbital_coordinates (float semiMajorAxis, float eccentricity, float eccentricAnomaly) {

  eccentricAnomaly *= rad;
  float true_Anomaly = 2 * atan(sqrt((1 + eccentricity) / (1 - eccentricity)) * tan(eccentricAnomaly / 2));
  true_Anomaly *= deg;
  true_Anomaly = calc_format_angle_deg (true_Anomaly);

  float radius = semiMajorAxis * (1 - (eccentricity * cos(eccentricAnomaly)));
  dist_object_to_sun = radius;
  #ifdef FEATURE_SERIAL_PLANETARY
    if (full) Serial.println("true_Anomaly:" + String(true_Anomaly, DEC));
    if (full) Serial.println("radius:" + String(radius, DEC));
  #endif

  calc_vector(0, true_Anomaly, radius, "to_rectangular"); // x = beta / y = true_Anomaly / z = radius
}

// =========================================================================
// object position
// =========================================================================
void get_object_position (int object_number, float jd, float jd_frac) {

  #ifdef FEATURE_SERIAL_PLANETARY 
    Serial.println(F("----------------------------------------------------"));
    ////Serial.println("Object: " + object_name[object_number]);
    Serial.println("Object: " + object_number);
  #endif

  float T = jd - 2451545;
  T += jd_frac;
  T /= 36525;
  #ifdef FEATURE_SERIAL_PLANETARY
    if (full) Serial.println("T:" + String(T, DEC));
  #endif

  float sidereal_time = calc_siderealTime (jd, jd_frac, lonPlanet);
  #ifdef FEATURE_SERIAL_PLANETARY
    if (full) Serial.println("ST:" + String(sidereal_time, DEC));
  #endif

  float semiMajorAxis = object_data[object_number][0] + (T * object_data[object_number][1]); // offset + T * delta
  float eccentricity = object_data[object_number][2] + (T * object_data[object_number][3]);
  float inclination = object_data[object_number][4] + (T * object_data[object_number][5]);
  float meanLongitude = object_data[object_number][6] + (T * object_data[object_number][7]);
  float longitudePerihelion = object_data[object_number][8] + (T * object_data[object_number][9]);
  float longitudeAscendingNode = object_data[object_number][10] + (T * object_data[object_number][11]);
  float meanAnomaly = meanLongitude - longitudePerihelion;
  float argumentPerihelion = longitudePerihelion - longitudeAscendingNode;

  #ifdef FEATURE_SERIAL_PLANETARY
    if (full) Serial.println("semiMajorAxis:" + String(semiMajorAxis, DEC));
    if (full) Serial.println("eccentricity:" + String(eccentricity, DEC));
  #endif
   
  inclination = calc_format_angle_deg (inclination);
  #ifdef FEATURE_SERIAL_PLANETARY
    if (full) Serial.println("inclination:" + String(inclination, DEC));
  #endif

  meanLongitude = calc_format_angle_deg (meanLongitude);
  longitudePerihelion = calc_format_angle_deg (longitudePerihelion);
  longitudeAscendingNode = calc_format_angle_deg (longitudeAscendingNode);
  meanAnomaly = calc_format_angle_deg (meanAnomaly);
  argumentPerihelion = calc_format_angle_deg (argumentPerihelion);
  
  #ifdef FEATURE_SERIAL_PLANETARY
    if (full) Serial.println("meanLongitude:" + String(meanLongitude, DEC));
    if (full) Serial.println("longitudePerihelion:" + String(longitudePerihelion, DEC));
    if (full) Serial.println("longitudeAscendingNode:" + String(longitudeAscendingNode, DEC));
    if (full) Serial.println("meanAnomaly:" + String(meanAnomaly, DEC));
    if (full) Serial.println("argumentPerihelion:" + String(argumentPerihelion, DEC));
  #endif  
  //---------------------------------
  float eccentricAnomaly = calc_eccentricAnomaly(meanAnomaly, eccentricity);
  eccentricAnomaly = calc_format_angle_deg (eccentricAnomaly);
  #ifdef FEATURE_SERIAL_PLANETARY
    if (full) Serial.println("eccentricAnomaly:" + String(eccentricAnomaly, DEC));
  #endif
  //---------------------------------
  //to orbital Coordinates:
  #ifdef FEATURE_SERIAL_PLANETARY
    if (full) Serial.println(F("orbital coordinates:"));
  #endif
  calc_orbital_coordinates (semiMajorAxis, eccentricity, eccentricAnomaly);
  //---------------------------------
  //to heliocentric ecliptic coordinates:
  rot_z (argumentPerihelion);
  rot_x (inclination);
  rot_z (longitudeAscendingNode);
  //---------------------------------
  if (object_number == 2) {//object earth

    x_earth = x_coord;
    y_earth = y_coord;
    z_earth = z_coord;
    //---------------------------------
    //calc the sun position from earth:
    
    calc_vector_subtract(x_earth, 0 , y_earth, 0, z_earth , 0);// earth - sun coordinates
    calc_vector(x_coord, y_coord, z_coord, "");

    #ifdef FEATURE_SERIAL_PLANETARY
      if (full) Serial.println(F("geocentric ecliptic results of sun:"));
      if (full) Serial.println(F("geocentric equatorial results of sun:"));
    #endif
    rot_x (eclipticAngle);//rotate x > earth ecliptic angle
    calc_vector(x_coord, y_coord, z_coord, "");
    dist_earth_to_sun = dist_earth_to_object;
    calc_azimuthal_position(ra, dec, lat, sidereal_time);
  }
  //---------------------------------
  if (object_number != 2) {//all other objects
    
    calc_vector_subtract(x_earth, x_coord , y_earth, y_coord, z_earth , z_coord);// earth - object coordinates
    calc_vector(x_coord, y_coord, z_coord, "");
    #ifdef FEATURE_SERIAL_PLANETARY
      if (full) Serial.println(F("geocentric ecliptic results of object:"));
      if (full) Serial.println(F("geocentric equatorial results of object:"));
    #endif
    rot_x (eclipticAngle);//rotate x > earth ecliptic angle
    calc_vector(x_coord, y_coord, z_coord, "");
    calc_azimuthal_position(ra, dec, lat, sidereal_time);
    calc_magnitude(object_number, dist_earth_to_object);
  }
}


//------------------------------------------------------------------------------------------------------------------
