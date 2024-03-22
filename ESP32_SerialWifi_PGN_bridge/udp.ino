void setupUDP()
{
  if (UDPforModules.listen(udpListenPort))
  {
    Serial.print("\r\nUDP Listening on IP:PORT "); Serial.print(myIP);
    Serial.print(":"); Serial.println(udpListenPort);

    UDPforModules.onPacket([](AsyncUDPPacket packet)      // this runs in a "loop", triggering each time a new packet arrives
    {
      SerialTeensy.write(packet.data(), packet.length());
      SerialTeensy.println();  // to signal end of PGN

      Serial.print("\r\nModules-w:9999->E32-s->T41 ");
      for (uint8_t i = 0; i < packet.length(); i++) {
        Serial.print(packet.data()[i]); Serial.print(" ");
      }
    });
  }
}