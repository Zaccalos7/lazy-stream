// swift-tools-version: 5.9
import PackageDescription

var products: [Product] = [.library(name: "OrbisCore", targets: ["OrbisCore"])]
var targets: [Target] = [
    .target(name: "AppleMetrics", publicHeadersPath: "include", linkerSettings: [.linkedFramework("IOKit", .when(platforms: [.macOS])), .linkedFramework("CoreFoundation", .when(platforms: [.macOS]))]),
    .target(name: "OrbisCore", dependencies: [.target(name: "AppleMetrics", condition: .when(platforms: [.macOS]))]),
    .testTarget(name: "OrbisCoreTests", dependencies: ["OrbisCore"])
]
#if os(macOS)
products.append(.executable(name: "OrbisStream", targets: ["OrbisMac"]))
targets.append(.executableTarget(name: "OrbisMac", dependencies: ["OrbisCore"]))
#endif
let package = Package(name: "OrbisStream", platforms: [.macOS(.v14)], products: products, targets: targets)
