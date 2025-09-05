import Foundation
import MLKit

// MARK: - Swift 에러 타입
public enum ExerciseSegmentError: Error {
    case initializationFailed
    case calibrationFailed
    case segmentCreationFailed
    case analysisFailed
    case unknownError(code: Int32)
    
    public var localizedDescription: String {
        switch self {
        case .initializationFailed:
            return "API 초기화에 실패했습니다."
        case .calibrationFailed:
            return "캘리브레이션에 실패했습니다."
        case .segmentCreationFailed:
            return "세그먼트 생성에 실패했습니다."
        case .analysisFailed:
            return "분석에 실패했습니다."
        case .unknownError(let code):
            return "알 수 없는 오류가 발생했습니다. (코드: \(code))"
        }
    }
}

// MARK: - Swift 래퍼 클래스
public class ExerciseSegmentManager {
    private var isInitialized = false
    private var isCalibrated = false
    private var calibrationData: CalibrationData?
    private var careJoints: [JointType] = []
    
    public init() {}
    
    deinit {
        if isInitialized {
            segment_destroy()
        }
    }
    
    // MARK: - 초기화
    public func initialize() throws {
        let result = segment_api_init()
        guard result == 0 else {
            throw ExerciseSegmentError.initializationFailed
        }
        isInitialized = true
    }
    
    // MARK: - 캘리브레이션
    public func calibrate(with basePose: Pose) throws {
        guard isInitialized else {
            throw ExerciseSegmentError.initializationFailed
        }
        
        let segmentPose = basePose.toSegmentPoseData()
        var calibration = CalibrationData()
        
        let result = segment_calibrate(&segmentPose, &calibration)
        guard result == 0 else {
            throw ExerciseSegmentError.calibrationFailed
        }
        
        self.calibrationData = calibration
        self.isCalibrated = true
    }
    
    // MARK: - 세그먼트 생성
    public func createSegment(
        startKeypose: Pose,
        endKeypose: Pose,
        careJoints: [JointType]
    ) throws {
        guard isCalibrated, let calibration = calibrationData else {
            throw ExerciseSegmentError.calibrationFailed
        }
        
        let startSegmentPose = startKeypose.toSegmentPoseData()
        let endSegmentPose = endKeypose.toSegmentPoseData()
        self.careJoints = careJoints
        
        // JointType 배열을 Int32 배열로 변환
        let jointIndices = careJoints.map { $0.rawValue }
        
        let result = jointIndices.withUnsafeBufferPointer { buffer in
            segment_create_with_indices(&startSegmentPose, &endSegmentPose, &calibration, buffer.baseAddress!, Int32(careJoints.count))
        }
        
        guard result == 0 else {
            throw ExerciseSegmentError.segmentCreationFailed
        }
    }
    
    // MARK: - 분석
    public func analyze(_ currentPose: Pose) throws -> SegmentOutput {
        guard isInitialized else {
            throw ExerciseSegmentError.initializationFailed
        }
        
        let segmentPose = currentPose.toSegmentPoseData()
        
        // Swift 친화적인 분석 함수 사용
        var progress: Float = 0.0
        var isComplete: Bool = false
        var similarity: Float = 0.0
        var corrections: [Point3D] = Array(repeating: Point3D(x: 0, y: 0, z: 0), count: 13)
        
        let result = corrections.withUnsafeMutableBufferPointer { buffer in
            segment_analyze_simple(&segmentPose, &progress, &isComplete, &similarity, buffer.baseAddress!)
        }
        
        guard result == 0 else {
            throw ExerciseSegmentError.analysisFailed
        }
        
        // SegmentOutput 구조체로 변환
        let output = SegmentOutput(
            progress: progress,
            completed: isComplete,
            similarity: similarity,
            corrections: corrections,
            timestamp: 0
        )
        
        return output
    }
    
    // MARK: - 상태 확인
    public var isReady: Bool {
        return isInitialized && isCalibrated
    }
}

// MARK: - Pose 확장
public extension Pose {
    func toSegmentPoseData() -> PoseData {
        // 13개 주요 관절 추출 (MLKit의 33개 랜드마크에서)
        var joints: [Point3D] = Array(repeating: Point3D(x: 0, y: 0, z: 0), count: 13)
        var confidence: [Float] = Array(repeating: 1.0, count: 13)
        
        // 관절 매핑 (MLKit -> ExerciseSegmentAPI)
        let jointMapping: [(PoseLandmarkType, Int)] = [
            (.nose, 0),
            (.leftShoulder, 1),
            (.rightShoulder, 2),
            (.leftElbow, 3),
            (.rightElbow, 4),
            (.leftWrist, 5),
            (.rightWrist, 6),
            (.leftHip, 7),
            (.rightHip, 8),
            (.leftKnee, 9),
            (.rightKnee, 10),
            (.leftAnkle, 11),
            (.rightAnkle, 12)
        ]
        
        for (landmarkType, index) in jointMapping {
            let landmark = landmark(ofType: landmarkType)
            joints[index] = Point3D(
                x: Float(landmark.position.x),
                y: Float(landmark.position.y),
                z: Float(landmark.position.z)
            )
            confidence[index] = Float(landmark.inFrameLikelihood)
        }
        
        return PoseData(
            joints: joints,
            confidence: confidence,
            timestamp: 0
        )
    }
}

// MARK: - 편의 함수들
public extension ExerciseSegmentManager {
    /// 간편한 세그먼트 생성 (기본 관절들 사용)
    func createSegmentWithDefaultJoints(
        startKeypose: Pose,
        endKeypose: Pose
    ) throws {
        let defaultJoints: [JointType] = [
            .leftShoulder, .rightShoulder,
            .leftElbow, .rightElbow,
            .leftWrist, .rightWrist,
            .leftHip, .rightHip
        ]
        
        try createSegment(
            startKeypose: startKeypose,
            endKeypose: endKeypose,
            careJoints: defaultJoints
        )
    }
    
    /// 분석 결과를 Swift 친화적인 형태로 변환
    func analyzeWithFeedback(_ currentPose: Pose) throws -> (progress: Float, isComplete: Bool, similarity: Float, corrections: [String]) {
        let output = try analyze(currentPose)
        
        var corrections: [String] = []
        
        // 교정 벡터를 기반으로 피드백 생성
        for (index, correction) in output.corrections.enumerated() {
            if correction.x > 0.1 || correction.y > 0.1 || correction.z > 0.1 {
                let jointName = getJointName(for: index)
                corrections.append("\(jointName)을 조금 더 \(getDirectionFeedback(correction)) 움직여주세요.")
            }
        }
        
        return (
            progress: output.progress,
            isComplete: output.isComplete,
            similarity: output.similarity,
            corrections: corrections
        )
    }
    
    private func getJointName(for index: Int) -> String {
        let jointNames = [
            "코", "왼쪽 눈", "오른쪽 눈", "왼쪽 귀", "오른쪽 귀",
            "왼쪽 어깨", "오른쪽 어깨", "왼쪽 팔꿈치", "오른쪽 팔꿈치",
            "왼쪽 손목", "오른쪽 손목", "왼쪽 엉덩이", "오른쪽 엉덩이"
        ]
        return index < jointNames.count ? jointNames[index] : "관절 \(index)"
    }
    
    private func getDirectionFeedback(_ correction: Point3D) -> String {
        var directions: [String] = []
        
        if abs(correction.x) > 0.1 {
            directions.append(correction.x > 0 ? "오른쪽으로" : "왼쪽으로")
        }
        if abs(correction.y) > 0.1 {
            directions.append(correction.y > 0 ? "위로" : "아래로")
        }
        if abs(correction.z) > 0.1 {
            directions.append(correction.z > 0 ? "앞으로" : "뒤로")
        }
        
        return directions.joined(separator: " ")
    }
}
