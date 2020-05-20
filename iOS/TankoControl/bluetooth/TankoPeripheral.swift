//
//  TankoPeripheral.swift
//  TankoControl
//
//  Created by Bernal Yescas, Francisco on 2/6/20.
//  Copyright © 2020 fby. All rights reserved.
//

import Foundation
import CoreBluetooth

public class TankoPeripheral: NSObject, Observable {
  
  public static let serviceUUID = "00000001-710e-4a5b-8d75-3e5b444bc3cf"
  public static let rxUUID = "00000002-710e-4a5b-8d75-3e5b444bc3cf"
  public static let txUUID = "00000003-710e-4a5b-8d75-3e5b444bc3cf"
  
  public let peripheral: CBPeripheral
  public private(set) var characteristics = [CBUUID: CBCharacteristic]()
  
  public init(with peripheral: CBPeripheral) {
    self.peripheral = peripheral
    super.init()
    self.peripheral.delegate = self
    self.peripheral.discoverServices(nil)
  }
  
  public func sendData(_ data: Data) {
    let uuid = CBUUID(string: TankoPeripheral.txUUID)
    guard let characteristic = self.characteristics[uuid] else {
      return
    }
    print("Send data: \(data.hexString())")
    peripheral.writeValue(data, for: characteristic, type: .withoutResponse)
  }
  
  // MARK: - Notifications
  
  public enum Events: String, CaseIterable {
    /// Notification posted when peripheral discovers a service
    case didDiscoverService = "Peripheral.DidDiscoverService"
    /// Notification posted when peripheral discovers a characteristic
    case didDiscoverCharacteristic = "Peripheral.DidDiscoverCharacteristic"
    /// Notification posted when peripheral received data from a characteristic
    case didReadDataFromCharacteristic = "Peripheral.DidReadDataFromCharacteristic"
    /// Notification posted when read peripheral's rssi
    case didReadRssi = "Peripheral.DidReadRssi"
    
    public func notificationName() -> Notification.Name {
      return Notification.Name(rawValue: self.rawValue)
    }
    
  }
  
  public func getNameList() -> [Notification.Name] {
    return Events.allCases.map { $0.notificationName() }
  }
  
  func post(_ event: Events, userInfo: [AnyHashable : Any]? = nil) {
    post(event.notificationName(), userInfo: userInfo)
  }
  
}


extension TankoPeripheral: CBPeripheralDelegate {
  
  public func peripheral(_ peripheral: CBPeripheral,
                         didDiscoverServices error: Error?) {
    guard let services = peripheral.services else {
      return
    }
    print("Did discover services for peripheral: \(peripheral.identifier.uuidString), count: \(services.count)")
    for service in services {
      peripheral.discoverCharacteristics(nil, for: service)
      post(.didDiscoverService, userInfo: ["peripheral": peripheral,
                                           "service": service])
    }
  }
  
  public func peripheral(_ peripheral: CBPeripheral,
                         didDiscoverCharacteristicsFor service: CBService,
                         error: Error?) {
    guard let characteristics = service.characteristics else {
      return
    }
    print("Did discover service: \(service.uuid.uuidString), characteristics count: \(characteristics.count)")
    for characteristic in characteristics {
      // save reference to characteristic
      self.characteristics[characteristic.uuid] = characteristic
      print("Discovered characteristic \(characteristic.uuid.uuidString)")
      
      if characteristic.properties.contains(.indicate) ||
        characteristic.properties.contains(.notify) {
        //subscribe for indications
        peripheral.setNotifyValue(true, for: characteristic)
        print("Enabling notifications for characteristic")
      }
      
      if characteristic.properties.contains(.read) {
        // read initial value
        print("Reading initial value for characteristic")
        peripheral.readValue(for: characteristic)
      }
      
      // notify
      post(.didDiscoverCharacteristic, userInfo: ["peripheral": peripheral,
                                                  "characteristic": characteristic])
    }
  }
  
  public func peripheral(_ peripheral: CBPeripheral,
                         didUpdateValueFor characteristic: CBCharacteristic,
                         error: Error?) {
    guard let value = characteristic.value else {
      return
    }
    print("Did receive data (\(value.count) bytes) for characteristic: \(characteristic.uuid.uuidString): \(value.hexString())")
    post(.didReadDataFromCharacteristic, userInfo: ["peripheral": peripheral,
                                                    "characteristic": characteristic,
                                                    "data": value])
  }
  
  public func peripheral(_ peripheral: CBPeripheral,
                         didWriteValueFor characteristic: CBCharacteristic,
                         error: Error?) {
    print("Did write value for characteristic: \(characteristic.uuid.uuidString)")
    if let error = error {
      print("\(error)")
    }
  }
  
  public func peripheral(_ peripheral: CBPeripheral,
                         didReadRSSI RSSI: NSNumber,
                         error: Error?) {
    print("Did read RSSI: \(RSSI.intValue)")
    post(.didReadRssi, userInfo: ["peripheral": peripheral,
                                  "rssi": RSSI])
  }
  
  public func peripheral(_ peripheral: CBPeripheral,
                         didModifyServices invalidatedServices: [CBService]) {
    print("Did modify services: \(invalidatedServices)")
    BLEScanner.shared.disconnectFromPeripheral(peripheral)
  }
  
}
