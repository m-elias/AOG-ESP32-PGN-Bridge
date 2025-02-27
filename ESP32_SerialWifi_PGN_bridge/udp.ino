void setupUDP()
{
  if (UDPforModules.listen(udpListenPort))
  {
    Serial.print("\r\nUDP Listening on IP:PORT "); Serial.print(myIP);
    Serial.print(":"); Serial.println(udpListenPort);

    UDPforModules.onPacket([](AsyncUDPPacket packet)      // this runs in a "loop", triggering each time a new packet arrives
    {
      if (packet.data()[0] == 128 && packet.data()[1] == 129) { // should be an AOG PGN
        SerialTeensy.write(packet.data(), packet.length());
        SerialTeensy.println();  // to signal end of PGN

        Serial.print("\r\nModules-w:9999->E32-s->T41 ");
        for (uint8_t i = 0; i < packet.length(); i++) {
          Serial.print(packet.data()[i]); Serial.print(" ");
        }
      } else {
        Serial.print("\r\n\nUDP Packet received but NOT valid PGN ([0]/[1] bytes != 128/129)\r\n");
      }
    });
  }
}