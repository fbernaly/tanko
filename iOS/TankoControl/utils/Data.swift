//
//  Data.swift
//  TankoControl
//
//  Created by Bernal Yescas, Francisco on 2/6/20.
//  Copyright © 2020 fby. All rights reserved.
//

import Foundation

public extension Data {
  
  /**
   Creates a string representation of a data by concatenating
   the hexadecimal representation of all bytes. The string _does not_ include
   the prefix '0x' that is commonly used to indicate hexadecimal representations.
   
   - returns: the hexadecimal representation of a byte array
   */
  func hexString() -> String {
    return map { String(format: "%02hhx", $0).uppercased() }.joined()
  }
  
  /**
   Create `Data` from hexadecimal string representation
   This creates a `Data` object from hex string.
   Note, if the string has any spaces or non-hex characters (e.g. starts with '<' and with a '>'),
   those are ignored and only hex characters are processed. If the string starts with '0x' it is removed.
   - parameter hexString: hexadecimal string.
   - returns: Data represented by this hexadecimal string.
   */
  init?(hexString: String) {
    var string = hexString
    if hexString.hasPrefix("0x") {
      let index = hexString.index(hexString.startIndex, offsetBy: 2)
      string = String(hexString[index...])
    }
    if hexString.hasPrefix("#") {
      let index = hexString.index(hexString.startIndex, offsetBy: 1)
      string = String(hexString[index...])
    }
    guard let regex = try? NSRegularExpression(pattern: "[0-9a-f]{1,2}",
                                               options: .caseInsensitive) else {
                                                return nil
    }
    self.init(capacity: string.count / 2)
    regex.enumerateMatches(in: string,
                           range: NSRange(string.startIndex...,
                                          in: string)) { match, _, _ in
                                            if let match = match {
                                              let byteString = (string as NSString).substring(with: match.range)
                                              if let num = UInt8(byteString, radix: 16) {
                                                self.append(num)
                                              }
                                            }
    }
    
    guard self.count > 0 else {
      return nil
    }
  }
  
}
