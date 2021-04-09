/**
 * Created by K. Suwatchai (Mobizt)
 * 
 * Email: k_suwatchai@hotmail.com
 * 
 * Github: https://github.com/mobizt
 * 
 * Copyright (c) 2021 mobizt
 *
*/

//This example shows how to backup database and send the Email

#include <WiFi.h>
#include <FirebaseESP32.h>

/*
   Required ESP Mail Client library for Arduino
   https://github.com/mobizt/ESP-Mail-Client
*/

#include <ESP_Mail_Client.h>

#define WIFI_SSID "WIFI_AP"
#define WIFI_PASSWORD "WIFI_PASSWORD"

#define FIREBASE_HOST "PROJECT_ID.firebaseio.com"

/** The database secret is obsoleted, please use other authentication methods, 
 * see examples in the Authentications folder. 
*/
#define FIREBASE_AUTH "DATABASE_SECRET"

//Define Firebase Data object
FirebaseData fbdo;


/* The smtp host name e.g. smtp.gmail.com for GMail or smtp.office365.com for Outlook */
#define SMTP_HOST "################"

/** The smtp port e.g. 
 * 25  or esp_mail_smtp_port_25
 * 465 or esp_mail_smtp_port_465
 * 587 or esp_mail_smtp_port_587
*/
#define SMTP_PORT 25

/* The sign in credentials */
#define AUTHOR_EMAIL "################"
#define AUTHOR_PASSWORD "################"

/* The SMTP Session object used for Email sending */
SMTPSession smtp;

/* Callback function to get the Email sending status */
void smtpCallback(SMTP_Status status);

void setup()
{

  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  //Print to see stack size and free memory
  UBaseType_t uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
  Serial.print("Stack: ");
  Serial.println(uxHighWaterMark);
  Serial.print("Heap: ");
  Serial.println(esp_get_free_heap_size());

  Serial.println();

  Serial.println("------------------------------------");
  Serial.println("Backup test...");


  if (!Firebase.backup(fbdo, StorageType::FLASH, "/PATH_TO_THE_NODE", "/PATH_TO_SAVE_FILE"))
  {
    Serial.println("FAILED");
    Serial.println("REASON: " + fbdo.fileTransferError());
    Serial.println("------------------------------------");
    Serial.println();
  }
  else
  {
    Serial.println("PASSED");
    Serial.println("SAVE PATH: " + fbdo.getBackupFilename());
    Serial.println("FILE SIZE: " + String(fbdo.getBackupFileSize()));
    Serial.println("------------------------------------");
    Serial.println();

    String filename = fbdo.getBackupFilename();

    if (fbdo.pauseFirebase(true))
    {

      //Send backup file via Email

      smtp.debug(1);

      /* Set the callback function to get the sending results */
      smtp.callback(smtpCallback);

      /* Declare the session config data */
      ESP_Mail_Session session;

      /* Set the session config */
      session.server.host_name = SMTP_HOST;
      session.server.port = SMTP_PORT;
      session.login.email = AUTHOR_EMAIL;
      session.login.password = AUTHOR_PASSWORD;
      session.login.user_domain = "mydomain.net";

      /* Declare the message class */
      SMTP_Message message;

      /* Set the message headers */
      message.sender.name = "ESP Mail";
      message.sender.email = AUTHOR_EMAIL;
      message.subject = "Firebase Database Backup File";
      message.addRecipient("Someone", "k_suwatchai@hotmail.com");

      message.text.content = "Firebase Database Backup File\r\nSent from ESP32";

      /** The Plain text message character set */
      message.text.charSet = "us-ascii";

      /** The content transfer encoding */
      message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

      /** The message priority */
      message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;

      /* The attachment data item */
      SMTP_Attachment att;

      /** Set the attachment info */
      att.descr.filename = filename.c_str();
      att.descr.mime = "application/octet-stream";
      String path = "/" + filename;

      att.file.path = path.c_str();
      att.file.storage_type = esp_mail_file_storage_type_flash;
      att.descr.transfer_encoding = Content_Transfer_Encoding::enc_base64;

      /* Add attachment to the message */
      message.addAttachment(att);

      /* Connect to server with the session config */
      if (!smtp.connect(&session))
        return;

      /* Start sending Email and close the session */
      if (!MailClient.sendMail(&smtp, &message))
        Serial.println("Error sending Email, " + smtp.errorReason());

      Serial.println();
    }
    else
    {
      Serial.println("Could not pause Firebase");
    }
  }

  //Quit Firebase and release all resources
  Firebase.end(fbdo);
}

void loop()
{
}

/* Callback function to get the Email sending status */
void smtpCallback(SMTP_Status status)
{
  /* Print the current status */
  Serial.println(status.info());

  /* Print the sending result */
  if (status.success())
  {
    Serial.println("----------------");
    Serial.printf("Message sent success: %d\n", status.completedCount());
    Serial.printf("Message sent failled: %d\n", status.failedCount());
    Serial.println("----------------\n");
    struct tm dt;

    for (size_t i = 0; i < smtp.sendingResult.size(); i++)
    {
      /* Get the result item */
      SMTP_Result result = smtp.sendingResult.getItem(i);
      localtime_r(&result.timesstamp, &dt);

      Serial.printf("Message No: %d\n", i + 1);
      Serial.printf("Status: %s\n", result.completed ? "success" : "failed");
      Serial.printf("Date/Time: %d/%d/%d %d:%d:%d\n", dt.tm_year + 1900, dt.tm_mon + 1, dt.tm_mday, dt.tm_hour, dt.tm_min, dt.tm_sec);
      Serial.printf("Recipient: %s\n", result.recipients);
      Serial.printf("Subject: %s\n", result.subject);
    }
    Serial.println("----------------\n");
  }
}
