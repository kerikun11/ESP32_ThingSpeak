# ESP32_ThingSpeak

ESP32 Library for ThingSpeak

## Install

This is a component of ESP-IDF.

```sh
cd YOUR_PROJECT_DIR
mkdir components # make local components directory if it does not exist
git clone https://github.com/kerikun11/ESP32_ThingSpeak components/thingspeak
```

## Example

```cpp
#include "ThingSpeak.h"

// define your parameter
#define THINGSPEAK_CHANNEL_ID "123456"
#define THINGSPEAK_API_KEY "0123456789ABCDEF"

extern "C" void app_main() {
  // wifi connection here

  // create a new feed
  ThingSpeak::Feed feed;
  feed.created_at = ThingSpeak::getTimeISO8601();
  feed.fields.push_back({1, 11.11f}); //< field1 = 11.11f
  feed.fields.push_back({2, 22.22f}); //< field2 = 22.22f
  feed.fields.push_back({3, 33.33f}); //< field3 = 33.33f

  // post the feed
  ThingSpeak client(THINGSPEAK_CHANNEL_ID, THINGSPEAK_API_KEY);
  client.post(feed);
}
```