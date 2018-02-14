#include "myUbidots.h"
#include "secrets.h"

Ubidots ubidots(TOKEN);

void initUbidots()
{
    ubidots.setDatasourceName(DATA_SOURCE_NAME);
    ubidots.setDatasourceTag(DATA_SOURCE_KEY);
}
