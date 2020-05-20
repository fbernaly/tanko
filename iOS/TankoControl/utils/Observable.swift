//
//  Observable.swift
//  TankoControl
//
//  Created by Bernal Yescas, Francisco on 2/7/20.
//  Copyright © 2020 fby. All rights reserved.
//

import Foundation

public protocol Observable {
  
  func getNameList() -> [Notification.Name]
  func addObserver(_ observer: Any, selector: Selector)
  func removeObserver(_ observer: Any)
  func post(_ name: Notification.Name, userInfo: [AnyHashable : Any]?)
  
}

extension Observable where Self: NSObject {
  
  public func addObserver(_ observer: Any, selector: Selector) {
    for name in getNameList() {
      NotificationCenter.default.addObserver(observer,
                                             selector: selector,
                                             name: name,
                                             object: self)
    }
  }
  
  public func removeObserver(_ observer: Any) {
    for name in getNameList() {
      NotificationCenter.default.removeObserver(observer,
                                                name: name,
                                                object: self)
    }
  }
  
  public func post(_ name: Notification.Name, userInfo: [AnyHashable : Any]?) {
    DispatchQueue.main.async {
      NotificationCenter.default.post(name: name,
                                      object: self,
                                      userInfo: userInfo)
    }
  }
  
}
