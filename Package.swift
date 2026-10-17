// swift-tools-version: 5.9
import PackageDescription
var targets: [Target] = [
    .target(name: "OrbisCore", linkerSettings: [.linkedFramework("IOKit", .when(platforms: [.macOS]))]),
    .testTarget(name: "OrbisCoreTests", dependencies: ["OrbisCore"])
]
var products: [Product] = []
#if os(macOS)
products.append(.executable(name: "OrbisStream", targets: ["OrbisMac"]))
targets += [
    .executableTarget(name: "OrbisMac", dependencies: ["OrbisCore"], resources: [.process("Resources")]),
    .testTarget(name: "OrbisMacTests", dependencies: ["OrbisMac", "OrbisCore"])
]
#endif
let package = Package(
    name: "OrbisStream", defaultLocalization: "it", platforms: [.macOS(.v14)],
    products: products,
    targets: targets
)
