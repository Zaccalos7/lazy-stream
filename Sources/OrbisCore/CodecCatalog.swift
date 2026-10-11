import Foundation

public enum CodecCatalog {
    public static let pixelFormats = ["yuv420p", "yuyv422", "rgb24", "bgr24", "yuv422p", "yuv444p", "yuv410p", "yuv411p", "gray", "monow", "monob", "pal8", "yuvj420p", "yuvj422p", "yuvj444p", "uyvy422", "uyyvyy411", "bgr8", "bgr4", "bgr4_byte", "rgb8", "rgb4", "rgb4_byte", "nv12", "nv21", "argb", "rgba", "abgr", "bgra", "gray16be", "gray16le", "yuv440p", "yuvj440p", "yuva420p", "rgb48be", "rgb48le", "rgb565be", "rgb565le", "rgb555be", "rgb555le", "bgr565be", "bgr565le", "bgr555be", "bgr555le", "yuv420p16le", "yuv420p16be", "yuv422p16le", "yuv422p16be", "yuv444p16le", "yuv444p16be", "rgb444le", "rgb444be", "bgr444le", "bgr444be", "ya8", "yuv420p10le"]
    public static let videoEncoders = ["h264_videotoolbox", "hevc_videotoolbox", "libx264", "libx265", "libaom-av1", "libsvtav1", "libvpx", "libvpx-vp9", "mpeg2video", "mpeg4", "vc1", "cinepak", "indeo3", "vp6", "vp6f", "h263", "flv", "libtheora", "wmv2", "mjpeg", "png", "dvvideo", "huffyuv", "ffv1", "prores_ks", "dnxhd", "vc2", "libwebp"]
    public static let audioEncoders = ["mp2", "libmp3lame", "aac", "ac3", "dca", "libvorbis", "flac", "alac", "libopus", "eac3", "truehd", "mlp", "pcm_s16le", "pcm_s24le", "pcm_f32le", "wmav2", "wmapro", "ape", "mpc8", "libopencore_amrnb", "libvo_amrwbenc"]
    public static let formats = ["flv", "mp4", "matroska", "avi", "mov", "webm", "mpegts"]
}
