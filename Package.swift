// swift-tools-version: 5.9
import PackageDescription
let package = Package(
    name: "OrbisStream", defaultLocalization: "it", platforms: [.macOS(.v14)],
    products: [.executable(name: "OrbisStream", targets: ["OrbisMac"])],
    targets: [
        .target(name: "OrbisCore", linkerSettings: [.linkedFramework("IOKit")]),
        .executableTarget(name: "OrbisMac", dependencies: ["OrbisCore"], resources: [.process("Resources")]),
        .testTarget(name: "OrbisCoreTests", dependencies: ["OrbisCore"]),
        .testTarget(name: "OrbisMacTests", dependencies: ["OrbisMac", "OrbisCore"])
    ]
)
