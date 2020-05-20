//
//  JoystickScene.swift
//  TankoControl
//
//  Created by Bernal Yescas, Francisco on 5/12/20.
//  Copyright © 2020 fby. All rights reserved.
//

import SpriteKit

protocol JoystickDelegate: NSObjectProtocol {
  func joystickDidMove(velocity: CGPoint)
  func joystickDidStop()
}

class JoystickScene: SKScene {
  
  lazy var joystick: TLAnalogJoystick = {
    let ratio: CGFloat = 0.6
    return TLAnalogJoystick(withDiameter: size.height / (1 + ratio), handleRatio: ratio)
  }()
  
  weak var joystickDelegate: JoystickDelegate?
  
  override func didMove(to view: SKView) {
    backgroundColor = .black
    physicsBody = SKPhysicsBody(edgeLoopFrom: frame)
    
    joystick.handleImage = UIImage(named: "jStick")
    joystick.baseImage = UIImage(named: "jSubstrate")
    joystick.position = CGPoint(x: frame.midX,
                                y: frame.midY)
    joystick.isHidden = false
    addChild(joystick)
    
    joystick.on(.move) { [unowned self] joystick in
      self.joystickDelegate?.joystickDidMove(velocity: joystick.velocity)
    }
    
    joystick.on(.end) { [unowned self] _ in
      self.joystickDelegate?.joystickDidStop()
    }

    view.isMultipleTouchEnabled = true
  }
  
}
