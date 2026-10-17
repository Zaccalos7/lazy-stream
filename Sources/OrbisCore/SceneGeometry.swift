import Foundation

public enum SceneGeometry {
    public static func placing(_ source: SceneItem, in scene: Scene, x: Int, y: Int) -> (scene: Scene, selection: UUID) {
        var result = scene
        var item = source
        if item.kind != .overlay && item.kind != .microphone,
           let index = result.items.indices.reversed().first(where: {
               let tile = result.items[$0]
               return tile.kind != .overlay && tile.kind != .microphone && x >= tile.x && y >= tile.y && x < tile.x + tile.width && y < tile.y + tile.height
           }) {
            let slot = result.items[index]
            item.id = slot.id; item.x = slot.x; item.y = slot.y; item.width = slot.width; item.height = slot.height
            result.items[index] = item
        } else {
            item.x = x; item.y = y
            item = constrain(item, width: result.width, height: result.height)
            result.items.append(item)
        }
        return (result, item.id)
    }
    public static func constrain(_ item: SceneItem, width: Int, height: Int) -> SceneItem {
        var result = item
        result.width = max(2, min(item.width, width)); result.width -= result.width % 2
        result.height = max(2, min(item.height, height)); result.height -= result.height % 2
        result.x = max(0, min(item.x, width - result.width)); result.y = max(0, min(item.y, height - result.height))
        return result
    }
    public static func snapped(_ item: SceneItem, in scene: Scene, tolerance: Int) -> SceneItem {
        var result = constrain(item, width: scene.width, height: scene.height)
        let other = scene.items.filter { $0.id != item.id && $0.kind != .microphone }
        let xs = [0, scene.width / 2, scene.width] + other.flatMap { [$0.x, $0.x + $0.width / 2, $0.x + $0.width] }
        let ys = [0, scene.height / 2, scene.height] + other.flatMap { [$0.y, $0.y + $0.height / 2, $0.y + $0.height] }
        func nearest(_ position: Int, size: Int, guides: [Int]) -> Int {
            let candidates = guides.flatMap { guide in [guide, guide - size / 2, guide - size] }
            guard let target = candidates.min(by: { abs($0 - position) < abs($1 - position) }), abs(target - position) <= tolerance else { return position }
            return target
        }
        result.x = nearest(result.x, size: result.width, guides: xs)
        result.y = nearest(result.y, size: result.height, guides: ys)
        return constrain(result, width: scene.width, height: scene.height)
    }
    public static func resized(_ scene: Scene, width: Int, height: Int) -> Scene {
        var result = scene
        result.width = max(2, min(7680, width)); result.width -= result.width % 2
        result.height = max(2, min(7680, height)); result.height -= result.height % 2
        let sx = Double(result.width) / Double(max(2, scene.width)), sy = Double(result.height) / Double(max(2, scene.height))
        result.items = scene.items.map { source in
            guard source.kind != .microphone else { return source }
            var item = source; item.x = Int((Double(source.x) * sx).rounded()); item.y = Int((Double(source.y) * sy).rounded())
            item.width = Int((Double(source.width) * sx).rounded()); item.height = Int((Double(source.height) * sy).rounded())
            return constrain(item, width: result.width, height: result.height)
        }
        return result
    }
    public static func applying(_ layout: Scene, to scene: Scene) -> Scene {
        var result = scene; result.width = layout.width; result.height = layout.height
        let sources = scene.items.filter { $0.kind != .overlay && $0.kind != .microphone }
        let microphones = scene.items.filter { $0.kind == .microphone }
        let slots = layout.items.filter { $0.kind != .overlay && $0.kind != .microphone }
        result.items = slots.enumerated().map { index, slot in
            var item = index < sources.count ? sources[index] : slot
            item.id = UUID(); item.x = slot.x; item.y = slot.y; item.width = slot.width; item.height = slot.height
            return item
        }
        if sources.count > slots.count {
            result.items += sources.dropFirst(slots.count).map { constrain($0, width: result.width, height: result.height) }
        }
        result.items += microphones
        result.items += layout.items.filter { $0.kind == .overlay }.map { var item = $0; item.id = UUID(); return item }
        return result
    }
}
