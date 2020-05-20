//
//  ViewController.swift
//  TankoControl
//
//  Created by Bernal Yescas, Francisco on 2/6/20.
//  Copyright © 2020 fby. All rights reserved.
//

import UIKit
import CoreBluetooth
import SpriteKit

class ViewController: UIViewController {
  
  @IBOutlet weak var channel0Slider: UISlider!
  @IBOutlet weak var channel1Slider: UISlider!
  @IBOutlet weak var channel2Slider: UISlider!
  @IBOutlet weak var channel3Slider: UISlider!
  @IBOutlet weak var channel4Slider: UISlider!
  @IBOutlet weak var channel5Slider: UISlider!
  @IBOutlet weak var channel6Slider: UISlider!
  
  @IBOutlet weak var scanningView: UIView!
  @IBOutlet weak var joystickView: SKView!
  
  @IBOutlet weak var initialPositionButton: UIButton!
  @IBOutlet weak var sleepPositionButton: UIButton!
  
  var tankoPeripheral: TankoPeripheral?
  var sent: String?
  var m1: Int = 0
  var m2: Int = 0
  
  override func viewDidLoad() {
    super.viewDidLoad()
    
    scanningView.isHidden = false
    scanningView.superview?.bringSubviewToFront(scanningView)
    
    BLEScanner.shared.addObserver(self, selector: #selector(self.receiveNotification(_:)))
    startScanning()
    
    let scene = JoystickScene(size: joystickView.bounds.size)
    scene.joystickDelegate = self
    joystickView.presentScene(scene)
  }
  
  @IBAction func sliderValueChanged(_ sender: UISlider) {
    sendCmd()
  }
  
  @IBAction func initialPositionButtonTapped(_ sender: UIButton) {
    m1 = 0
    m2 = 0
    let arr: [UInt8] = [90, 0, 120, 90, 90, 90, 90, 0x00, 100, 100]
    let data = Data(arr)
    sendData(data)
    parseData(data)
  }
  
  @IBAction func sleepPositionButtonTapped(_ sender: UIButton) {
    m1 = 0
    m2 = 0
    let arr: [UInt8] = [90, 45, 120, 90, 45, 90, 90, 0x00, 100, 100]
    let data = Data(arr)
    sendData(data)
    parseData(data)
  }
  
  @objc private func receiveNotification (_ notification: Foundation.Notification) {
    if let event = BLEScanner.Events(rawValue: notification.name.rawValue) {
      switch event {
      case .peripheralFound:
        if let data = notification.userInfo,
          let peripheral = data["peripheral"] as? CBPeripheral {
          BLEScanner.shared.stopScanningForPeripheral()
          BLEScanner.shared.connectToPeripheral(peripheral)
        }
        
      case .didConnectToPeripheral:
        if let data = notification.userInfo,
          let peripheral = data["peripheral"] as? CBPeripheral {
          scanningView.isHidden = true
          tankoPeripheral = TankoPeripheral(with: peripheral)
          tankoPeripheral?.addObserver(self, selector: #selector(self.receiveNotification(_:)))
        }
        
      case .didDisconnectFromPeripheral:
        tankoPeripheral = nil
        scanningView.isHidden = false
        
      default:
        break
      }
    }
    
    if let event = TankoPeripheral.Events(rawValue: notification.name.rawValue) {
      switch event {
      case .didReadDataFromCharacteristic:
        if let userInfo = notification.userInfo,
          let data = userInfo["data"] as? Data {
          parseData(data)
        }
        
      default:
        break
      }
    }
  }
  
  public func startScanning() {
    let uuid = CBUUID(string: TankoPeripheral.serviceUUID)
    BLEScanner.shared.scanForPeripherals(withServices: [uuid],
                                         names: nil,
                                         timeout: -1)
  }
  
  func parseData(_ data: Data) {
    channel0Slider.value = Float(Int(data[0]))
    channel1Slider.value = Float(Int(data[1]))
    channel2Slider.value = Float(Int(data[2]))
    channel3Slider.value = Float(Int(data[3]))
    channel4Slider.value = Float(Int(data[4]))
    channel5Slider.value = Float(Int(data[5]))
    channel6Slider.value = Float(Int(data[6]))
  }
  
  func sendCmd() {
    var arr: [UInt8] = []
    arr.append(UInt8(Int(channel0Slider.value)))
    arr.append(UInt8(Int(channel1Slider.value)))
    arr.append(UInt8(Int(channel2Slider.value)))
    arr.append(UInt8(Int(channel3Slider.value)))
    arr.append(UInt8(Int(channel4Slider.value)))
    arr.append(UInt8(Int(channel5Slider.value)))
    arr.append(UInt8(Int(channel6Slider.value)))
    arr.append(0x00)
    arr.append(UInt8(m1 + 100))
    arr.append(UInt8(m2 + 100))
    let data = Data(arr)
    sendData(data)
  }
  
  func sendData(_ data: Data) {
    let sent = data.hexString()
    guard self.sent != sent else {
      return
    }
    self.sent = sent
    tankoPeripheral?.sendData(data)
  }
  
}

// MARK: - JoystickDelegate

extension ViewController: JoystickDelegate {
  
  func joystickDidStop() {
    m1 = 0
    m2 = 0
    sendCmd()
  }
  
  func joystickDidMove(velocity: CGPoint) {
    let x = velocity.x
    let y = velocity.y
    
    let s = (Int((x * x + y * y).squareRoot() * 100 + 10) / 25) * 25
    
    var t = atan2(y, x)
    if t < 0 {
        t += (.pi * 2)
    }
    t = t * 180 / .pi
    
    m1 = 0
    m2 = 0
    switch t {
    case 0..<15:
        m1 = -s
        m2 = s
    case 15..<75:
        m1 = 0
        m2 = s
    case 75..<105:
        m1 = s
        m2 = s
    case 105..<165:
        m1 = s
        m2 = 0
    case 165..<195:
        m1 = s
        m2 = -s
    case 195..<255:
        m1 = 0
        m2 = -s
    case 255..<285:
        m1 = -s
        m2 = -s
    case 285..<345:
        m1 = -s
        m2 = 0
    case 345..<361:
        m1 = -s
        m2 = s
        
    default:
        break
    }
    
    sendCmd()
  }
  
  func clamp(_ value : CGFloat) -> CGFloat {
    let min: CGFloat = -0.5
    let max: CGFloat = 0.5
    return min > value ? min
      : max < value ? max
      : value
  }
  
}
