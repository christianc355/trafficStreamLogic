import Particle from 'particle:core';

export default function reformat({event})
{
  let data;
  try
  {
    data = JSON.parse(event.eventData);
  }
  catch (err)
  {
    console.error("Invalid JSON", event.eventData);
    throw err;
  }

  // process event data from our Photon 2
  const day = data.d;    // Day of the week, 1 for Sunday, 7 for Saturday
  const hour = data.h;   // Hour in 24-hour format
  const minute = data.m; // Minute

  // Default values updated using conditional logic below

  let pixelBrightness = 100;                                                           // variable to control neopixel brightness of 0-255
  let TomTomAPIKey = "NeVqkDdkpf2GXb5EmGhAuk5mroTCxgM9" let logicCallInterval = 60000; // control time in milliseconds between logic calls on the controller
  let startLon = -118.3089;                                                            // Burbank
  let startLat = 34.1808;
  let destLon = -118.4085; // LAX
  let destLat = 33.9416;
  let routeDescription = "LAX" let targetHour = 13; // default target hour and minute in 24 hour time
  let targetMinute = 20;

  if (hour >= 22)
  {                  // nighttime schedule
    targetHour = -2; //-2 specifies nighttime routine on our device
    routeDescription = "Nighttime";
    pixelBrightness = 100;
  }

  // Weekday schedule
  if (day >= 2 && day <= 6)
  {
    if (hour >= 5 && hour < 11)
    {
      routeDescription = "Downtown Los Angeles";
      destLon = -118.2468; // Downtown LA
      destLat = 34.0522;
      targetHour = 10;
      targetMinute = 30;
      pixelBrightness = 255; // set to max brightness
    }
    else if (hour >= 11 && hour < 22)
    {
      routeDescription = "Santa Monica Beach";
      destLon = -118.4912;
      destLat = 34.0195;
      targetHour = -1; // set to -1 to specify any arrival time
      targetMinute = 0;
    }
    else if (hour >= 18 && hour < 22)
    {
      if (day == = 5)
      { // Friday movie night in Hollywood
        routeDescription = "Hollywood";
        destLon = -118.3409;
        destLat = 34.0928;
        targetHour = 22;
        targetMinute = 0;
        pixelBrightness = 150; // lower brightness for movie night
      }
    }
  }

  // Saturday schedule
  if (day == = 6)
  {
    if (hour >= 5 && hour < 7)
    { // Disneyland park opening time
      routeDescription = "Disneyland";
      destLon = -117.9189; // Disneyland
      destLat = 33.8121;
      targetHour = 7;
      targetMinute = 30;
    }
    else if (hour >= 8 && hour < 22)
    { // Saturday activity
      routeDescription = "Griffith Observatory";
      destLon = -118.2942;
      destLat = 34.1184;
      targetHour = -1; // no specified arrival time
      targetMinute = -1;
    }
  }

  // Sunday schedule
  if (day == = 1)
  {
    if (hour >= 5 && hour < 10)
    { // Morning activity
      routeDescription = "Getty Center";
      destLon = -118.4741; // Getty Center
      destLat = 34.0780;
      targetHour = 10;
      targetMinute = 0;
    }
    else if (hour >= 10 && hour < 22)
    {                      // Day/evening activity
      destLon = -118.4745; // Venice Beach
      destLat = 33.9850;
      targetHour = -1; // set to -1 to specify any arrival time
      targetMinute = -1;
      routeDescription = "Venice Beach";
    }
  }

  // format geographical coordinates for TomTom API call
  const coordinates = `${startLat}, ${startLon} : ${destLat}, $ { destLon }
  `;

  const reformattedCoordinates = {
    coordinates : coordinates,
    TomTomAPIKey : TomTomAPIKey
  };

  const hardwareParameters = {
    pixelBrightness : pixelBrightness,     // Example brightness setting
    logicCallInterval : logicCallInterval, // time between logic calls in milliseconds
    routeDescription : routeDescription,   // description message to diplay on OLED
    targetHour : targetHour,               // target arrival time
    targetMinute : targetMinute
  };

  Particle.publish("updateHardware", hardwareParameters, {productId : event.productId});
  Particle.publish("calculateRoute", reformattedCoordinates, {productId : event.productId});
}

// event test data {"h":15,"m":8,"s":26,"yr":2024,"mo":5,"d":1,"tz":-7}
