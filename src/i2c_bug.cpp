/**
 * @file i2c_bug.cpp
 * @author sylque
 * @brief I2C bug on M5StickCPlus2 when using IMU + BLE
 * @version 1.0
 * @date 2024-04-10
 *
 *
 * @Hardwares: M5StickCPlus2
 * @Dependent Library: M5StickCPlus2
 *
 * IMPORTANT NOTE: in order to compile this program, you need to patch the
 * M5Unified library in order to make "StickCP2.Imu._imu_instance" a public
 * member of IMU_Class.hpp (instead of private). In order to do this:
 * 1. Compile this program under Platformio
 * 2. You will get this compile error: "src/i2c_bug.cpp:41:28: error:
 * 'std::unique_ptr<m5::IMU_Base> m5::IMU_Class::_imu_instance [2]' is private
 * within this context"
 * 3. In the code, right-click "_imu_instance" and choose "Go to Definition"
 * 4. Make the definition of "_imu_instance" public instead of private
 */

#include <BLEDevice.h>
#include <M5StickCPlus2.h>

#include <utility/imu/MPU6886_Class.hpp>

//------------------------------------------------------------------------------

void setup() {
  //===========================================
  // Init M5
  //===========================================
  auto cfg = M5.config();
  StickCP2.begin(cfg);
  StickCP2.Display.setRotation(3);
  StickCP2.Display.setTextSize(3);

  //===========================================
  // Modify the default IMU settings
  //===========================================

  // Get the IMU instance. YOU WILL GET A COMPILE ERROR HERE. To resolve it,
  // change the M5Unified library to make "_imu_instance" a public property
  // (instead of private). See also the IMPORTANT NOTE at the top of this page.
  auto* imu = StickCP2.Imu._imu_instance[0].get();

  // Set Accel range to ±2g and gyro range to ±245 °/s
  assert(imu->writeRegister8(m5::MPU6886_Class::REG_GYRO_CONFIG,
                             m5::MPU6886_Class::GFS_250DPS << 3));
  delay(10);
  assert(imu->writeRegister8(m5::MPU6886_Class::REG_ACCEL_CONFIG,
                             m5::MPU6886_Class::AFS_2G << 3));
  delay(10);

  // Set the sampling rate to 1kHz
  assert(imu->writeRegister8(m5::MPU6886_Class::REG_SMPLRT_DIV, 0x00));
  delay(10);

  // Set heavy filter on both gyro and accel
  assert(imu->writeRegister8(m5::MPU6886_Class::REG_CONFIG, 0x05));
  delay(10);
  assert(imu->writeRegister8(m5::MPU6886_Class::REG_ACCEL_CONFIG2,
                             5 << 0 | 0 << 3 | 0 << 4));
  delay(10);

  //===========================================
  // Setup a BLE service
  //===========================================

  // Init BLE
  BLEDevice::init("I2C Bug");

  // Create a BLE service
  BLEServer* bleServer = BLEDevice::createServer();
  BLEService* bleService = bleServer->createService("6A5B");

  // Add a bunch of BLE characteristics ("1000" to "1006")
  for (int i = 1000; i <= 1006; ++i) {
    auto characUuid = std::to_string(i);
    bleService->createCharacteristic(characUuid.c_str(), 0);
  }

  // Start BLE
  bleService->start();
  BLEDevice::startAdvertising();
}

//------------------------------------------------------------------------------

void loop(void) {
  // Read IMU data
  StickCP2.Imu.update();

  // Display a message every second, to show that the program is still alive
  static unsigned int seconds = 0;
  static auto t = millis();
  if (millis() - t > 1000) {
    StickCP2.Display.setCursor(60, 60);
    StickCP2.Display.printf("%d      ", seconds);
    Serial.printf("%d ", seconds++);
    t = millis();
  }
}

//------------------------------------------------------------------------------
