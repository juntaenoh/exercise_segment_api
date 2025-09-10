import ExerciseSegmentAPI
import Foundation
import GoogleMLKit

class ExerciseSegmentManager: ObservableObject {
    @Published var isInitialized = false
    @Published var isCalibrated = false
    @Published var isSegmentCreated = false

    private var api: ExerciseSegmentAPI?
    private var calibration: CalibrationData?

    func initialize() -> Bool {
        do {
            api = ExerciseSegmentAPI()
            try api?.initialize()
            isInitialized = true
            return true
        } catch {
            print("API 초기화 실패: \(error)")
            isInitialized = false
            return false
        }
    }

    func cleanup() {
        api?.cleanup()
        api = nil
        isInitialized = false
        isCalibrated = false
        isSegmentCreated = false
        calibration = nil
    }

    func calibrate() {
        guard let api = api, isInitialized else {
            print("API가 초기화되지 않았습니다.")
            return
        }

        // 기본 포즈로 캘리브레이션
        let basePose = createSamplePose()

        do {
            calibration = try api.calibrate(basePose: basePose)
            isCalibrated = true
            print("캘리브레이션 성공!")
        } catch {
            print("캘리브레이션 실패: \(error)")
            isCalibrated = false
        }
    }

    func createSegment() -> Bool {
        guard let api = api, let calibration = calibration, isInitialized && isCalibrated else {
            print("API 초기화 및 캘리브레이션이 필요합니다.")
            return false
        }

        let startPose = createSquatStartPose()
        let endPose = createSquatEndPose()
        let careJoints: [JointType] = [.leftKnee, .rightKnee, .leftHip, .rightHip]

        do {
            try api.createSegment(
                startKeypose: startPose,
                endKeypose: endPose,
                calibration: calibration,
                careJoints: careJoints
            )
            isSegmentCreated = true
            print("세그먼트 생성 성공!")
            return true
        } catch {
            print("세그먼트 생성 실패: \(error)")
            isSegmentCreated = false
            return false
        }
    }

    func analyze(_ pose: Pose) -> AnalysisResult? {
        guard let api = api, isInitialized && isSegmentCreated else {
            print("API 초기화 및 세그먼트 생성이 필요합니다.")
            return nil
        }

        // Google ML Kit Pose를 PoseData로 변환 (변환 없이 직접 사용)
        let poseData = convertPoseToPoseData(pose)
        let input = SegmentInput(rawPose: poseData)

        do {
            let output = try api.analyze(input: input)
            return AnalysisResult(
                progress: output.progress,
                completed: output.completed,
                similarity: output.similarity,
                corrections: output.corrections,
                timestamp: output.timestamp
            )
        } catch {
            print("분석 실패: \(error)")
            return nil
        }
    }

    func resetSegment() {
        guard let api = api, isSegmentCreated else { return }

        do {
            try api.resetSegment()
            print("세그먼트 리셋 완료!")
        } catch {
            print("세그먼트 리셋 실패: \(error)")
        }
    }

    func destroySegment() {
        guard let api = api else { return }

        api.destroySegment()
        isSegmentCreated = false
        print("세그먼트 해제 완료!")
    }

    // MARK: - Helper Methods

    /// Google ML Kit Pose를 PoseData로 변환 (변환 없이 직접 매핑)
    private func convertPoseToPoseData(_ pose: Pose) -> PoseData {
        var landmarks: [PoseLandmark] = []

        // Google ML Kit의 13개 관절을 순서대로 매핑
        let landmarkTypes: [PoseLandmarkType] = [
            .nose, .leftShoulder, .rightShoulder, .leftElbow, .rightElbow,
            .leftWrist, .rightWrist, .leftHip, .rightHip, .leftKnee,
            .rightKnee, .leftAnkle, .rightAnkle,
        ]

        for landmarkType in landmarkTypes {
            let landmark = pose.landmark(ofType: landmarkType)
            let poseLandmark = PoseLandmark(
                position: Point2D(x: Float(landmark.position.x), y: Float(landmark.position.y)),
                inFrameLikelihood: landmark.inFrameLikelihood
            )
            landmarks.append(poseLandmark)
        }

        let timestamp = UInt64(Date().timeIntervalSince1970 * 1000)
        return PoseData(landmarks: landmarks, timestamp: timestamp)
    }

    private func createSamplePose() -> PoseData {
        let landmarks = [
            PoseLandmark(position: Point2D(x: 0.0, y: -10.0), inFrameLikelihood: 0.9),  // nose
            PoseLandmark(position: Point2D(x: -20.0, y: 0.0), inFrameLikelihood: 0.9),  // left_shoulder
            PoseLandmark(position: Point2D(x: 20.0, y: 0.0), inFrameLikelihood: 0.9),  // right_shoulder
            PoseLandmark(position: Point2D(x: -30.0, y: 20.0), inFrameLikelihood: 0.9),  // left_elbow
            PoseLandmark(position: Point2D(x: 30.0, y: 20.0), inFrameLikelihood: 0.9),  // right_elbow
            PoseLandmark(position: Point2D(x: -40.0, y: 40.0), inFrameLikelihood: 0.9),  // left_wrist
            PoseLandmark(position: Point2D(x: 40.0, y: 40.0), inFrameLikelihood: 0.9),  // right_wrist
            PoseLandmark(position: Point2D(x: -10.0, y: 50.0), inFrameLikelihood: 0.9),  // left_hip
            PoseLandmark(position: Point2D(x: 10.0, y: 50.0), inFrameLikelihood: 0.9),  // right_hip
            PoseLandmark(position: Point2D(x: -10.0, y: 80.0), inFrameLikelihood: 0.9),  // left_knee
            PoseLandmark(position: Point2D(x: 10.0, y: 80.0), inFrameLikelihood: 0.9),  // right_knee
            PoseLandmark(position: Point2D(x: -10.0, y: 110.0), inFrameLikelihood: 0.9),  // left_ankle
            PoseLandmark(position: Point2D(x: 10.0, y: 110.0), inFrameLikelihood: 0.9),  // right_ankle
        ]

        let timestamp = UInt64(Date().timeIntervalSince1970 * 1000)
        return PoseData(landmarks: landmarks, timestamp: timestamp)
    }

    private func createSquatStartPose() -> PoseData {
        return createSamplePose()
    }

    private func createSquatEndPose() -> PoseData {
        var landmarks = createSamplePose().landmarks

        // 무릎을 구부리고 골반을 낮춤
        landmarks[9] = PoseLandmark(
            position: Point2D(x: landmarks[9].position.x, y: landmarks[9].position.y + 30.0),
            inFrameLikelihood: 0.9)  // left_knee
        landmarks[10] = PoseLandmark(
            position: Point2D(x: landmarks[10].position.x, y: landmarks[10].position.y + 30.0),
            inFrameLikelihood: 0.9)  // right_knee
        landmarks[7] = PoseLandmark(
            position: Point2D(x: landmarks[7].position.x, y: landmarks[7].position.y + 20.0),
            inFrameLikelihood: 0.9)  // left_hip
        landmarks[8] = PoseLandmark(
            position: Point2D(x: landmarks[8].position.x, y: landmarks[8].position.y + 20.0),
            inFrameLikelihood: 0.9)  // right_hip

        let timestamp = UInt64(Date().timeIntervalSince1970 * 1000)
        return PoseData(landmarks: landmarks, timestamp: timestamp)
    }
}

struct AnalysisResult {
    let progress: Float
    let completed: Bool
    let similarity: Float
    let corrections: [Point2D]
    let timestamp: UInt64
}
