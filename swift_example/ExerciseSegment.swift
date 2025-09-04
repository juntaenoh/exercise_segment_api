//
//  ExerciseSegment.swift
//  Exercise Segment API Swift Wrapper
//
//  Created by Exercise Segment API Team
//  Copyright © 2024. All rights reserved.
//

import Foundation

// MARK: - Data Structures

/// 3D 좌표점
public struct Point3D {
    public let x: Float
    public let y: Float
    public let z: Float
    
    public init(x: Float, y: Float, z: Float) {
        self.x = x
        self.y = y
        self.z = z
    }
}

/// 관절 타입
public enum JointType: Int32 {
    case nose = 0
    case leftShoulder = 1
    case rightShoulder = 2
    case leftElbow = 3
    case rightElbow = 4
    case leftWrist = 5
    case rightWrist = 6
    case leftHip = 7
    case rightHip = 8
    case leftKnee = 9
    case rightKnee = 10
    case leftAnkle = 11
    case rightAnkle = 12
}

/// 포즈 데이터
public struct PoseData {
    public let joints: [Point3D]
    public let confidence: [Float]
    public let timestamp: UInt64
    
    public init(joints: [Point3D], confidence: [Float], timestamp: UInt64) {
        self.joints = joints
        self.confidence = confidence
        self.timestamp = timestamp
    }
}

/// 캘리브레이션 데이터
public struct CalibrationData {
    public let scaleFactor: Float
    public let centerOffset: Point3D
    public let isCalibrated: Bool
    public let calibrationQuality: Float
    
    public init(scaleFactor: Float, centerOffset: Point3D, isCalibrated: Bool, calibrationQuality: Float) {
        self.scaleFactor = scaleFactor
        self.centerOffset = centerOffset
        self.isCalibrated = isCalibrated
        self.calibrationQuality = calibrationQuality
    }
}

/// 세그먼트 입력
public struct SegmentInput {
    public let rawPose: PoseData
    
    public init(rawPose: PoseData) {
        self.rawPose = rawPose
    }
}

/// 세그먼트 출력
public struct SegmentOutput {
    public let progress: Float
    public let completed: Bool
    public let similarity: Float
    public let corrections: [Point3D]
    public let timestamp: UInt64
    
    public init(progress: Float, completed: Bool, similarity: Float, corrections: [Point3D], timestamp: UInt64) {
        self.progress = progress
        self.completed = completed
        self.similarity = similarity
        self.corrections = corrections
        self.timestamp = timestamp
    }
}

// MARK: - Error Types

public enum ExerciseSegmentError: Error {
    case notInitialized
    case invalidPose
    case calibrationFailed
    case segmentNotCreated
    case invalidParameter
    case memoryAllocation
    case unknown(Int32)
    
    init(errorCode: Int32) {
        switch errorCode {
        case -1: self = .notInitialized
        case -2: self = .invalidPose
        case -3: self = .calibrationFailed
        case -4: self = .segmentNotCreated
        case -5: self = .invalidParameter
        case -6: self = .memoryAllocation
        default: self = .unknown(errorCode)
        }
    }
}

// MARK: - Main API Class

public class ExerciseSegmentAPI {
    
    // MARK: - Properties
    
    private var isInitialized: Bool = false
    private var isSegmentCreated: Bool = false
    
    // MARK: - Initialization
    
    public init() {}
    
    deinit {
        cleanup()
    }
    
    // MARK: - Public Methods
    
    /// API 초기화
    public func initialize() throws {
        let result = segment_api_init()
        guard result == 0 else {
            throw ExerciseSegmentError(errorCode: result)
        }
        isInitialized = true
    }
    
    /// API 정리
    public func cleanup() {
        if isInitialized {
            segment_api_cleanup()
            isInitialized = false
            isSegmentCreated = false
        }
    }
    
    /// 사용자 캘리브레이션
    public func calibrate(basePose: PoseData) throws -> CalibrationData {
        guard isInitialized else {
            throw ExerciseSegmentError.notInitialized
        }
        
        var cBasePose = convertToCPoseData(basePose)
        var cCalibration = CCalibrationData()
        
        let result = segment_calibrate(&cBasePose, &cCalibration)
        guard result == 0 else {
            throw ExerciseSegmentError(errorCode: result)
        }
        
        return convertFromCCalibrationData(cCalibration)
    }
    
    /// 캘리브레이션 유효성 검사
    public func validateCalibration(_ calibration: CalibrationData) -> Bool {
        let cCalibration = convertToCCalibrationData(calibration)
        return segment_validate_calibration(&cCalibration)
    }
    
    /// 운동 세그먼트 생성
    public func createSegment(
        startKeypose: PoseData,
        endKeypose: PoseData,
        calibration: CalibrationData,
        careJoints: [JointType]
    ) throws {
        guard isInitialized else {
            throw ExerciseSegmentError.notInitialized
        }
        
        var cStartPose = convertToCPoseData(startKeypose)
        var cEndPose = convertToCPoseData(endKeypose)
        var cCalibration = convertToCCalibrationData(calibration)
        
        let careJointsArray = careJoints.map { $0.rawValue }
        let result = careJointsArray.withUnsafeBufferPointer { buffer in
            segment_create(&cStartPose, &cEndPose, &cCalibration, buffer.baseAddress, Int32(careJoints.count))
        }
        
        guard result == 0 else {
            throw ExerciseSegmentError(errorCode: result)
        }
        
        isSegmentCreated = true
    }
    
    /// 실시간 포즈 분석
    public func analyze(input: SegmentInput) throws -> SegmentOutput {
        guard isInitialized && isSegmentCreated else {
            throw ExerciseSegmentError.notInitialized
        }
        
        var cInput = convertToCSegmentInput(input)
        let cOutput = segment_analyze(&cInput)
        
        return convertFromCSegmentOutput(cOutput)
    }
    
    /// 세그먼트 리셋
    public func resetSegment() throws {
        guard isInitialized && isSegmentCreated else {
            throw ExerciseSegmentError.notInitialized
        }
        
        let result = segment_reset()
        guard result == 0 else {
            throw ExerciseSegmentError(errorCode: result)
        }
    }
    
    /// 세그먼트 해제
    public func destroySegment() {
        if isSegmentCreated {
            segment_destroy()
            isSegmentCreated = false
        }
    }
    
    /// 포즈 유사도 계산
    public func calculateSimilarity(pose1: PoseData, pose2: PoseData) -> Float {
        var cPose1 = convertToCPoseData(pose1)
        var cPose2 = convertToCPoseData(pose2)
        return segment_calculate_similarity(&cPose1, &cPose2)
    }
    
    /// 포즈 유효성 검사
    public func validatePose(_ pose: PoseData) -> Bool {
        var cPose = convertToCPoseData(pose)
        return segment_validate_pose(&cPose)
    }
    
    /// 에러 메시지 가져오기
    public func getErrorMessage(errorCode: Int32) -> String {
        let cString = segment_get_error_message(errorCode)
        return String(cString: cString)
    }
}

// MARK: - Private Conversion Methods

private extension ExerciseSegmentAPI {
    
    func convertToCPoseData(_ pose: PoseData) -> CPoseData {
        var cJoints: [CPoint3D] = []
        var cConfidence: [Float] = []
        
        for i in 0..<13 {
            if i < pose.joints.count {
                let joint = pose.joints[i]
                cJoints.append(CPoint3D(x: joint.x, y: joint.y, z: joint.z))
            } else {
                cJoints.append(CPoint3D(x: 0, y: 0, z: 0))
            }
            
            if i < pose.confidence.count {
                cConfidence.append(pose.confidence[i])
            } else {
                cConfidence.append(0.0)
            }
        }
        
        return CPoseData(
            joints: cJoints,
            confidence: cConfidence,
            timestamp: pose.timestamp
        )
    }
    
    func convertFromCCalibrationData(_ cCalibration: CCalibrationData) -> CalibrationData {
        let centerOffset = Point3D(
            x: cCalibration.center_offset.x,
            y: cCalibration.center_offset.y,
            z: cCalibration.center_offset.z
        )
        
        return CalibrationData(
            scaleFactor: cCalibration.scale_factor,
            centerOffset: centerOffset,
            isCalibrated: cCalibration.is_calibrated,
            calibrationQuality: cCalibration.calibration_quality
        )
    }
    
    func convertToCCalibrationData(_ calibration: CalibrationData) -> CCalibrationData {
        let centerOffset = CPoint3D(
            x: calibration.centerOffset.x,
            y: calibration.centerOffset.y,
            z: calibration.centerOffset.z
        )
        
        return CCalibrationData(
            scale_factor: calibration.scaleFactor,
            center_offset: centerOffset,
            is_calibrated: calibration.isCalibrated,
            calibration_quality: calibration.calibrationQuality
        )
    }
    
    func convertToCSegmentInput(_ input: SegmentInput) -> CSegmentInput {
        let cRawPose = convertToCPoseData(input.rawPose)
        return CSegmentInput(raw_pose: cRawPose)
    }
    
    func convertFromCSegmentOutput(_ cOutput: CSegmentOutput) -> SegmentOutput {
        var corrections: [Point3D] = []
        for i in 0..<13 {
            let correction = cOutput.corrections[i]
            corrections.append(Point3D(x: correction.x, y: correction.y, z: correction.z))
        }
        
        return SegmentOutput(
            progress: cOutput.progress,
            completed: cOutput.completed,
            similarity: cOutput.similarity,
            corrections: corrections,
            timestamp: cOutput.timestamp
        )
    }
}

// MARK: - C Structure Definitions

private struct CPoint3D {
    let x: Float
    let y: Float
    let z: Float
}

private struct CPoseData {
    let joints: [CPoint3D]
    let confidence: [Float]
    let timestamp: UInt64
}

private struct CCalibrationData {
    let scale_factor: Float
    let center_offset: CPoint3D
    let is_calibrated: Bool
    let calibration_quality: Float
}

private struct CSegmentInput {
    let raw_pose: CPoseData
}

private struct CSegmentOutput {
    let progress: Float
    let completed: Bool
    let similarity: Float
    let corrections: [CPoint3D]
    let timestamp: UInt64
}

// MARK: - C Function Declarations

@_silgen_name("segment_api_init")
private func segment_api_init() -> Int32

@_silgen_name("segment_api_cleanup")
private func segment_api_cleanup()

@_silgen_name("segment_calibrate")
private func segment_calibrate(_ basePose: UnsafePointer<CPoseData>, _ outCalibration: UnsafeMutablePointer<CCalibrationData>) -> Int32

@_silgen_name("segment_validate_calibration")
private func segment_validate_calibration(_ calibration: UnsafePointer<CCalibrationData>) -> Bool

@_silgen_name("segment_create")
private func segment_create(_ startKeypose: UnsafePointer<CPoseData>, _ endKeypose: UnsafePointer<CPoseData>, _ calibration: UnsafePointer<CCalibrationData>, _ careJoints: UnsafePointer<Int32>, _ careJointCount: Int32) -> Int32

@_silgen_name("segment_analyze")
private func segment_analyze(_ input: UnsafePointer<CSegmentInput>) -> CSegmentOutput

@_silgen_name("segment_reset")
private func segment_reset() -> Int32

@_silgen_name("segment_destroy")
private func segment_destroy()

@_silgen_name("segment_calculate_similarity")
private func segment_calculate_similarity(_ pose1: UnsafePointer<CPoseData>, _ pose2: UnsafePointer<CPoseData>) -> Float

@_silgen_name("segment_validate_pose")
private func segment_validate_pose(_ pose: UnsafePointer<CPoseData>) -> Bool

@_silgen_name("segment_get_error_message")
private func segment_get_error_message(_ errorCode: Int32) -> UnsafePointer<CChar>
