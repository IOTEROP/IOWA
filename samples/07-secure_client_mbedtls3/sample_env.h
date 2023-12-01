/* Specifc sample values */

// Device detail
// NOTE: This name should be unique. Could be a serial number, an IMEI, etc...
#define SAMPLE_ENDPOINT_NAME       "prod123456_psk"

// LwM2M Server details
#define SAMPLE_SERVER_URI          "coaps://datagram-ingress.alaska.ioterop.com"
#define SAMPLE_SERVER_SHORT_ID     1234
#define SAMPLE_SERVER_LIFETIME     50

// PSK security credentials
#define SAMPLE_PSK_IDENTITY     "MyTestID"              // Should be a string
#define SAMPLE_PSK_KEY          {'T','e','s','t','K','E','Y'}            // base64:VGVzdEtFWQ==