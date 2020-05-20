//
//  BLEScanner.swift
//  TankoControl
//
//  Created by Bernal Yescas, Francisco on 2/6/20.
//  Copyright © 2020 fby. All rights reserved.
//

import Foundation
import CoreBluetooth

public class BLEScanner: NSObject, Observable {
  
  public static let shared = BLEScanner()
  
  public private(set) var centralManager: CBCentralManager?
  public var discoveredPeripherals = [String: CBPeripheral]()
  public var allowReconnection: Bool = true
  
  var timer: Timer?
  var timeout: TimeInterval?
  var serviceUUIDs: [CBUUID]?
  var names: [String]?
  
  override init() {
    super.init()
    centralManager = CBCentralManager(delegate: self,
                                      queue: nil,
                                      options: [CBCentralManagerOptionShowPowerAlertKey: true])
  }
  
  public func start() {
    if centralManager == nil {
      centralManager = CBCentralManager(delegate: self,
                                        queue: nil,
                                        options: [CBCentralManagerOptionShowPowerAlertKey: true])
    }
  }
  
  public func restartManager() {
    guard centralManager?.state == .poweredOff else {
      return
    }
    centralManager = CBCentralManager(delegate: self,
                                      queue: nil,
                                      options: [CBCentralManagerOptionShowPowerAlertKey: true])
    discoveredPeripherals.removeAll()
  }
  
  // MARK: - Scanning
  
  // scanInterval <= 0 means no timeout
  public func scanForPeripherals(withServices serviceUUIDs: [CBUUID]?,
                                 names: [String]?,
                                 timeout: TimeInterval) {
    self.serviceUUIDs = serviceUUIDs
    self.names = names
    self.timeout = timeout > 0 ? timeout : nil
    guard centralManager?.state == .poweredOn else {
      return
    }
    print("Scan for peripherals service UUIDs:\(serviceUUIDs ?? []))")
    discoveredPeripherals.removeAll()
    centralManager?.delegate = self
    centralManager?.scanForPeripherals(withServices: serviceUUIDs,
                                       options: [CBCentralManagerScanOptionAllowDuplicatesKey: false])
    if let timeout = self.timeout {
      timer = Timer.scheduledTimer(timeInterval: timeout,
                                   target: self,
                                   selector: #selector(timeoutFindPeripheral),
                                   userInfo: nil,
                                   repeats: false)
    }
    post(.searchingForPeripheral)
  }
  
  public func stopScanningForPeripheral() {
    guard centralManager?.state == .poweredOn,
      centralManager?.isScanning == true else {
        return
    }
    print("Stop scaning for peripherals")
    centralManager?.stopScan()
    timer?.invalidate()
  }
  
  public func resetScanningForPeripheral() {
    print("Reset scaning for peripherals")
    stopScanningForPeripheral()
    post(.resetSearchingForPeripheral)
    DispatchQueue.main.asyncAfter(deadline: .now() + 1) {
      self.scanForPeripherals(withServices: self.serviceUUIDs,
                              names: self.names,
                              timeout: self.timeout ?? 0)
    }
  }
  
  // MARK: - Connection
  
  public func connectToPeripheral(_ peripheral: CBPeripheral) {
    connectToPeripheral(withUUID: peripheral.identifier.uuidString)
  }
  
  public func connectToPeripheral(withUUID uuid: String) {
    guard let peripheral = discoveredPeripherals[uuid] else {
      print("Peripheral with uuid \'\(uuid)\' not found")
      return
    }
    print("Connect to peripheral with uuid: \(uuid)")
    centralManager?.connect(peripheral, options: nil)
    timer?.invalidate()
    if let timeout = self.timeout {
      timer = Timer.scheduledTimer(timeInterval: timeout,
                                   target: self,
                                   selector: #selector(timeoutConnectToPeripheral(_:)),
                                   userInfo: peripheral,
                                   repeats: false)
    }
    post(.connectingToPeripheral, userInfo: ["peripheral": peripheral])
  }
  
  public func disconnectFromPeripheral(_ peripheral: CBPeripheral) {
    guard centralManager?.state == .poweredOn else {
      return
    }
    if centralManager?.isScanning == true {
      centralManager?.stopScan()
    }
    print("Disconnect from peripheral: \(peripheral.name ?? "<name>") UUID: \(peripheral.identifier.uuidString)")
    centralManager?.cancelPeripheralConnection(peripheral)
    discoveredPeripherals.removeAll()
    timer?.invalidate()
    timer = nil
  }
  
  // MARK: - Timeouts
  
  @objc func timeoutFindPeripheral() {
    print("Peripheral scanning timed out")
    timer?.invalidate()
    centralManager?.stopScan()
    post(.peripheralNotFound)
  }
  
  @objc func timeoutConnectToPeripheral(_ timer: Timer) {
    print("Peripheral connection timed out")
    guard let peripheral = timer.userInfo as? CBPeripheral else {
      return
    }
    centralManager?.cancelPeripheralConnection(peripheral)
    post(.didFailToConnectToPeripheral, userInfo: ["peripheral": peripheral])
  }
  
  // MARK: - Notifications
  
  public enum Events: String, CaseIterable {
    /// Notification posted when BLE changes status.
    case didUpdateState = "BLEScanner.DidUpdateState"
    /// Notification posted when manager starts searching for peripheral.
    case searchingForPeripheral = "BLEScanner.SearchingForPeripheral"
    /// Notification posted when manager resets scanning for peripheral.
    case resetSearchingForPeripheral = "BLEScanner.ResetSearchingForPeripheral"
    /// Notification posted when manager finds a peripheral
    case peripheralFound = "BLEScanner.PeripheralFound"
    /// Notification posted when manager does not find a peripheral
    case peripheralNotFound = "BLEScanner.PeripheralNotFound"
    /// Notification posted when manager starts connecting to a peripheral.
    case connectingToPeripheral = "BLEScanner.ConnectingToPeripheral"
    /// Notification posted when manager connects to a peripheral.
    case didConnectToPeripheral = "BLEScanner.DidConnectToPeripheral"
    /// Notification posted when manager fails to connect to a peripheral.
    case didFailToConnectToPeripheral = "BLEScanner.DidFailToConnectToPeripheral"
    /// Notification posted when manager did disconnect from peripheral.
    case didDisconnectFromPeripheral = "BLEScanner.DidDisconnectFromPeripheral"
    
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

// MARK: - CBCentralManagerDelegate

extension BLEScanner: CBCentralManagerDelegate {
  
  public func centralManagerDidUpdateState(_ central: CBCentralManager) {
    print("Central Manager did update state: \(central.state)")
    timer?.invalidate()
    if central.state == .poweredOn {
      resetScanningForPeripheral()
    }
    post(.didUpdateState, userInfo: ["state": central.state,
                                     "description": central.state])
  }
  
  public func centralManager(_ central: CBCentralManager,
                             didDiscover peripheral: CBPeripheral,
                             advertisementData: [String: Any],
                             rssi RSSI: NSNumber) {
    if let names = self.names {
      guard let pName = peripheral.name,
        names.contains(pName) else {
          return
      }
    }
    if discoveredPeripherals[peripheral.identifier.uuidString] != nil {
      discoveredPeripherals[peripheral.identifier.uuidString] = peripheral
      return
    }
    print(">> Central Manager did discover peripheral: \(peripheral.name ?? "<name>") UUID: \(peripheral.identifier.uuidString) rssi: \(RSSI) \n \(advertisementData)")
    timer?.invalidate()
    discoveredPeripherals[peripheral.identifier.uuidString] = peripheral
    post(.peripheralFound, userInfo: ["peripheral": peripheral,
                                      "advertisementData": advertisementData,
                                      "rssi": RSSI])
  }
  
  public func centralManager(_ central: CBCentralManager,
                             didConnect peripheral: CBPeripheral) {
    print("Central Manager did connect to peripheral: \(peripheral.identifier.uuidString)")
    timer?.invalidate()
    post(.didConnectToPeripheral, userInfo: ["peripheral": peripheral])
  }
  
  public func centralManager(_ central: CBCentralManager,
                             didFailToConnect peripheral: CBPeripheral,
                             error: Error?) {
    print("Central Manager did fail to connect to peripheral: \(peripheral.identifier.uuidString) error: \(String(describing: error))")
    post(.didFailToConnectToPeripheral, userInfo: ["peripheral": peripheral])
  }
  
  public func centralManager(_ central: CBCentralManager,
                             didDisconnectPeripheral peripheral: CBPeripheral,
                             error: Error?) {
    print("Central Manager did disconnect from peripheral: \(peripheral.identifier.uuidString)")
    post(.didDisconnectFromPeripheral, userInfo: ["peripheral": peripheral])
    if central.state == .poweredOn,
      allowReconnection {
      resetScanningForPeripheral()
    }
  }
  
}
