// swift-tools-version: 5.9
import PackageDescription

let package = Package(
    name: "OrbisStream",
    platforms: [.macOS(.v14)],
    products: [.executable(name: "OrbisStream", targets: ["OrbisMac"]), .library(name: "OrbisCore", targets: ["OrbisCore"])],
    targets: [
        .target(name: "AppleMetrics", publicHeadersPath: "include"),
        .target(name: "OrbisCore", dependencies: [.target(name: "AppleMetrics", condition: .when(platforms: [.macOS]))]),
        .executableTarget(name: "OrbisMac", dependencies: ["OrbisCore"]),
        .testTarget(name: "OrbisCoreTests", dependencies: ["OrbisCore"])
    ]
)
