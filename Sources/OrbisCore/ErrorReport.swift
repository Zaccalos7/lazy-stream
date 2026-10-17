import Foundation

public enum ErrorReport {
    public static func redact(_ text: String, keys: [String]) -> String {
        var secrets = Set<String>()
        for key in keys where !key.isEmpty {
            secrets.insert(key); secrets.insert(key.removingPercentEncoding ?? key)
            if let base = key.split(separator: "?", maxSplits: 1).first, !base.isEmpty {
                secrets.insert(String(base)); secrets.insert(String(base).removingPercentEncoding ?? String(base))
            }
            if let query = key.split(separator: "?", maxSplits: 1).dropFirst().first {
                for parameter in query.split(separator: "&") {
                    let value = parameter.split(separator: "=", maxSplits: 1, omittingEmptySubsequences: false)
                    if value.count == 2, value[1].count >= 8 {
                        secrets.insert(String(value[1])); secrets.insert(String(value[1]).removingPercentEncoding ?? String(value[1]))
                    }
                }
            }
        }
        var result = text
        for secret in secrets.filter({ !$0.isEmpty }).sorted(by: { $0.count > $1.count }) {
            result = result.replacingOccurrences(of: secret, with: "****")
        }
        result = result.replacingOccurrences(of: #"(?i)rtmps?://[^\s'"<>]+"#, with: "[stream URL hidden]", options: .regularExpression)
        return result
    }
    /// Stay below the QR byte limit without splitting a Unicode scalar.
    public static func parts(_ text: String, maxBytes: Int = 600) -> [String] {
        let limit = max(4, maxBytes)
        var parts: [String] = []; var part = ""; var count = 0
        for scalar in text.unicodeScalars {
            let value = String(scalar); let bytes = value.utf8.count
            if count + bytes > limit { parts.append(part); part = ""; count = 0 }
            part += value; count += bytes
        }
        if !part.isEmpty { parts.append(part) }
        return parts
    }
}
