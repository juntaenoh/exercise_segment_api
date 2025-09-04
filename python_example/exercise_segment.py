"""
Exercise Segment API Python Wrapper
C API를 Python에서 사용할 수 있도록 래핑한 모듈
"""

import ctypes
import ctypes.util
import platform
import os
from typing import List, Optional, Tuple
from dataclasses import dataclass
from enum import IntEnum

# 라이브러리 로드
def _load_library():
    """플랫폼에 따라 적절한 라이브러리를 로드합니다."""
    system = platform.system()
    
    if system == "Darwin":  # macOS
        lib_path = "../build/libexercise_segment.dylib"
    elif system == "Linux":
        # Docker 환경에서는 /usr/local/lib에서 찾기
        lib_path = "/usr/local/lib/libexercise_segment.so"
        if not os.path.exists(lib_path):
            lib_path = "../build/libexercise_segment.so"
    elif system == "Windows":
        lib_path = "../build/exercise_segment.dll"
    else:
        raise OSError(f"지원하지 않는 플랫폼: {system}")
    
    if not os.path.exists(lib_path):
        raise FileNotFoundError(f"라이브러리를 찾을 수 없습니다: {lib_path}")
    
    return ctypes.CDLL(lib_path)

# C 라이브러리 로드
_lib = _load_library()

# 상수 정의
class JointType(IntEnum):
    NOSE = 0
    LEFT_SHOULDER = 1
    RIGHT_SHOULDER = 2
    LEFT_ELBOW = 3
    RIGHT_ELBOW = 4
    LEFT_WRIST = 5
    RIGHT_WRIST = 6
    LEFT_HIP = 7
    RIGHT_HIP = 8
    LEFT_KNEE = 9
    RIGHT_KNEE = 10
    LEFT_ANKLE = 11
    RIGHT_ANKLE = 12

class SegmentErrorCode(IntEnum):
    OK = 0
    NOT_INITIALIZED = -1
    INVALID_POSE = -2
    CALIBRATION_FAILED = -3
    SEGMENT_NOT_CREATED = -4
    INVALID_PARAMETER = -5
    MEMORY_ALLOCATION = -6

# 데이터 클래스 정의
@dataclass
class Point3D:
    x: float
    y: float
    z: float

@dataclass
class PoseData:
    joints: List[Point3D]
    confidence: List[float]
    timestamp: int

@dataclass
class CalibrationData:
    scale_factor: float
    center_offset: Point3D
    is_calibrated: bool
    calibration_quality: float

@dataclass
class SegmentInput:
    raw_pose: PoseData

@dataclass
class SegmentOutput:
    progress: float
    completed: bool
    similarity: float
    corrections: List[Point3D]
    timestamp: int

# C 구조체 정의
class CPoint3D(ctypes.Structure):
    _fields_ = [
        ("x", ctypes.c_float),
        ("y", ctypes.c_float),
        ("z", ctypes.c_float)
    ]

class CPoseData(ctypes.Structure):
    _fields_ = [
        ("joints", CPoint3D * 13),
        ("confidence", ctypes.c_float * 13),
        ("timestamp", ctypes.c_uint64)
    ]

class CCalibrationData(ctypes.Structure):
    _fields_ = [
        ("scale_factor", ctypes.c_float),
        ("center_offset", CPoint3D),
        ("is_calibrated", ctypes.c_bool),
        ("calibration_quality", ctypes.c_float)
    ]

class CSegmentInput(ctypes.Structure):
    _fields_ = [
        ("raw_pose", CPoseData)
    ]

class CSegmentOutput(ctypes.Structure):
    _fields_ = [
        ("progress", ctypes.c_float),
        ("completed", ctypes.c_bool),
        ("similarity", ctypes.c_float),
        ("corrections", CPoint3D * 13),
        ("timestamp", ctypes.c_uint64)
    ]

# 함수 시그니처 설정
_lib.segment_api_init.restype = ctypes.c_int
_lib.segment_calibrate.argtypes = [ctypes.POINTER(CPoseData), ctypes.POINTER(CCalibrationData)]
_lib.segment_calibrate.restype = ctypes.c_int
_lib.segment_validate_calibration.argtypes = [ctypes.POINTER(CCalibrationData)]
_lib.segment_validate_calibration.restype = ctypes.c_bool
_lib.segment_create.argtypes = [
    ctypes.POINTER(CPoseData), 
    ctypes.POINTER(CPoseData), 
    ctypes.POINTER(CCalibrationData),
    ctypes.POINTER(ctypes.c_int32),
    ctypes.c_int32
]
_lib.segment_create.restype = ctypes.c_int
_lib.segment_analyze.argtypes = [ctypes.POINTER(CSegmentInput)]
_lib.segment_analyze.restype = CSegmentOutput
_lib.segment_reset.restype = ctypes.c_int
_lib.segment_calculate_similarity.argtypes = [ctypes.POINTER(CPoseData), ctypes.POINTER(CPoseData)]
_lib.segment_calculate_similarity.restype = ctypes.c_float
_lib.segment_validate_pose.argtypes = [ctypes.POINTER(CPoseData)]
_lib.segment_validate_pose.restype = ctypes.c_bool
_lib.segment_get_error_message.argtypes = [ctypes.c_int]
_lib.segment_get_error_message.restype = ctypes.c_char_p

# 변환 함수들
def _to_c_point3d(point: Point3D) -> CPoint3D:
    return CPoint3D(x=point.x, y=point.y, z=point.z)

def _from_c_point3d(c_point: CPoint3D) -> Point3D:
    return Point3D(x=c_point.x, y=c_point.y, z=c_point.z)

def _to_c_pose_data(pose: PoseData) -> CPoseData:
    joints = (CPoint3D * 13)()
    confidence = (ctypes.c_float * 13)()
    
    for i in range(13):
        if i < len(pose.joints):
            joints[i] = _to_c_point3d(pose.joints[i])
        else:
            joints[i] = CPoint3D(x=0.0, y=0.0, z=0.0)
        
        if i < len(pose.confidence):
            confidence[i] = pose.confidence[i]
        else:
            confidence[i] = 0.0
    
    return CPoseData(joints=joints, confidence=confidence, timestamp=pose.timestamp)

def _from_c_pose_data(c_pose: CPoseData) -> PoseData:
    joints = [_from_c_point3d(c_pose.joints[i]) for i in range(13)]
    confidence = [c_pose.confidence[i] for i in range(13)]
    return PoseData(joints=joints, confidence=confidence, timestamp=c_pose.timestamp)

def _to_c_calibration_data(calibration: CalibrationData) -> CCalibrationData:
    return CCalibrationData(
        scale_factor=calibration.scale_factor,
        center_offset=_to_c_point3d(calibration.center_offset),
        is_calibrated=calibration.is_calibrated,
        calibration_quality=calibration.calibration_quality
    )

def _from_c_calibration_data(c_calibration: CCalibrationData) -> CalibrationData:
    return CalibrationData(
        scale_factor=c_calibration.scale_factor,
        center_offset=_from_c_point3d(c_calibration.center_offset),
        is_calibrated=c_calibration.is_calibrated,
        calibration_quality=c_calibration.calibration_quality
    )

def _to_c_segment_input(input_data: SegmentInput) -> CSegmentInput:
    return CSegmentInput(raw_pose=_to_c_pose_data(input_data.raw_pose))

def _from_c_segment_output(c_output: CSegmentOutput) -> SegmentOutput:
    corrections = [_from_c_point3d(c_output.corrections[i]) for i in range(13)]
    return SegmentOutput(
        progress=c_output.progress,
        completed=c_output.completed,
        similarity=c_output.similarity,
        corrections=corrections,
        timestamp=c_output.timestamp
    )

# 메인 API 클래스
class ExerciseSegmentAPI:
    """Exercise Segment API의 Python 래퍼 클래스"""
    
    def __init__(self):
        self._initialized = False
        self._segment_created = False
    
    def initialize(self) -> bool:
        """API를 초기화합니다."""
        result = _lib.segment_api_init()
        self._initialized = (result == SegmentErrorCode.OK)
        return self._initialized
    
    def cleanup(self):
        """API를 정리합니다."""
        if self._initialized:
            _lib.segment_api_cleanup()
            self._initialized = False
            self._segment_created = False
    
    def calibrate(self, base_pose: PoseData) -> Optional[CalibrationData]:
        """사용자 캘리브레이션을 수행합니다."""
        if not self._initialized:
            return None
        
        c_base_pose = _to_c_pose_data(base_pose)
        c_calibration = CCalibrationData()
        
        result = _lib.segment_calibrate(ctypes.byref(c_base_pose), ctypes.byref(c_calibration))
        
        if result == SegmentErrorCode.OK:
            return _from_c_calibration_data(c_calibration)
        return None
    
    def validate_calibration(self, calibration: CalibrationData) -> bool:
        """캘리브레이션 데이터의 유효성을 검사합니다."""
        c_calibration = _to_c_calibration_data(calibration)
        return _lib.segment_validate_calibration(ctypes.byref(c_calibration))
    
    def create_segment(self, start_keypose: PoseData, end_keypose: PoseData, 
                      calibration: CalibrationData, care_joints: List[JointType]) -> bool:
        """운동 세그먼트를 생성합니다."""
        if not self._initialized:
            return False
        
        c_start_pose = _to_c_pose_data(start_keypose)
        c_end_pose = _to_c_pose_data(end_keypose)
        c_calibration = _to_c_calibration_data(calibration)
        
        # 관절 배열을 C 배열로 변환
        care_joints_array = (ctypes.c_int32 * len(care_joints))()
        for i, joint in enumerate(care_joints):
            care_joints_array[i] = joint.value
        
        result = _lib.segment_create(
            ctypes.byref(c_start_pose),
            ctypes.byref(c_end_pose),
            ctypes.byref(c_calibration),
            care_joints_array,
            len(care_joints)
        )
        
        if result == SegmentErrorCode.OK:
            self._segment_created = True
            return True
        return False
    
    def analyze(self, input_data: SegmentInput) -> Optional[SegmentOutput]:
        """실시간 포즈 분석을 수행합니다."""
        if not self._initialized or not self._segment_created:
            return None
        
        c_input = _to_c_segment_input(input_data)
        c_output = _lib.segment_analyze(ctypes.byref(c_input))
        
        return _from_c_segment_output(c_output)
    
    def reset_segment(self) -> bool:
        """세그먼트를 리셋합니다."""
        if not self._initialized or not self._segment_created:
            return False
        
        result = _lib.segment_reset()
        return result == SegmentErrorCode.OK
    
    def destroy_segment(self):
        """세그먼트를 해제합니다."""
        if self._segment_created:
            _lib.segment_destroy()
            self._segment_created = False
    
    def calculate_similarity(self, pose1: PoseData, pose2: PoseData) -> float:
        """두 포즈 간의 유사도를 계산합니다."""
        c_pose1 = _to_c_pose_data(pose1)
        c_pose2 = _to_c_pose_data(pose2)
        return _lib.segment_calculate_similarity(ctypes.byref(c_pose1), ctypes.byref(c_pose2))
    
    def validate_pose(self, pose: PoseData) -> bool:
        """포즈 데이터의 유효성을 검사합니다."""
        c_pose = _to_c_pose_data(pose)
        return _lib.segment_validate_pose(ctypes.byref(c_pose))
    
    def get_error_message(self, error_code: int) -> str:
        """에러 코드에 해당하는 메시지를 반환합니다."""
        c_string = _lib.segment_get_error_message(error_code)
        return c_string.decode('utf-8') if c_string else "Unknown error"
    
    def __enter__(self):
        """Context manager 진입"""
        self.initialize()
        return self
    
    def __exit__(self, exc_type, exc_val, exc_tb):
        """Context manager 종료"""
        self.cleanup()

# 편의 함수들
def create_sample_pose() -> PoseData:
    """샘플 포즈를 생성합니다."""
    joints = [
        Point3D(0.0, -10.0, 0.0),      # nose
        Point3D(-20.0, 0.0, 0.0),      # left_shoulder
        Point3D(20.0, 0.0, 0.0),       # right_shoulder
        Point3D(-30.0, 20.0, 0.0),     # left_elbow
        Point3D(30.0, 20.0, 0.0),      # right_elbow
        Point3D(-40.0, 40.0, 0.0),     # left_wrist
        Point3D(40.0, 40.0, 0.0),      # right_wrist
        Point3D(-10.0, 50.0, 0.0),     # left_hip
        Point3D(10.0, 50.0, 0.0),      # right_hip
        Point3D(-10.0, 80.0, 0.0),     # left_knee
        Point3D(10.0, 80.0, 0.0),      # right_knee
        Point3D(-10.0, 110.0, 0.0),    # left_ankle
        Point3D(10.0, 110.0, 0.0)      # right_ankle
    ]
    
    confidence = [0.9] * 13
    timestamp = int(__import__('time').time() * 1000)
    
    return PoseData(joints=joints, confidence=confidence, timestamp=timestamp)

def create_squat_start_pose() -> PoseData:
    """스쿼트 시작 포즈를 생성합니다."""
    return create_sample_pose()

def create_squat_end_pose() -> PoseData:
    """스쿼트 종료 포즈를 생성합니다."""
    joints = [
        Point3D(0.0, -10.0, 0.0),      # nose
        Point3D(-20.0, 0.0, 0.0),      # left_shoulder
        Point3D(20.0, 0.0, 0.0),       # right_shoulder
        Point3D(-30.0, 20.0, 0.0),     # left_elbow
        Point3D(30.0, 20.0, 0.0),      # right_elbow
        Point3D(-40.0, 40.0, 0.0),     # left_wrist
        Point3D(40.0, 40.0, 0.0),      # right_wrist
        Point3D(-10.0, 70.0, 0.0),     # left_hip (낮춤)
        Point3D(10.0, 70.0, 0.0),      # right_hip (낮춤)
        Point3D(-10.0, 110.0, 0.0),    # left_knee (구부림)
        Point3D(10.0, 110.0, 0.0),     # right_knee (구부림)
        Point3D(-10.0, 110.0, 0.0),    # left_ankle
        Point3D(10.0, 110.0, 0.0)      # right_ankle
    ]
    
    confidence = [0.9] * 13
    timestamp = int(__import__('time').time() * 1000)
    
    return PoseData(joints=joints, confidence=confidence, timestamp=timestamp)

def create_intermediate_pose(progress: float) -> PoseData:
    """진행도에 따른 중간 포즈를 생성합니다."""
    start_joints = create_squat_start_pose().joints
    end_joints = create_squat_end_pose().joints
    
    intermediate_joints = []
    for i in range(13):
        start_joint = start_joints[i]
        end_joint = end_joints[i]
        
        x = start_joint.x + progress * (end_joint.x - start_joint.x)
        y = start_joint.y + progress * (end_joint.y - start_joint.y)
        z = start_joint.z + progress * (end_joint.z - start_joint.z)
        
        intermediate_joints.append(Point3D(x, y, z))
    
    confidence = [0.9] * 13
    timestamp = int(__import__('time').time() * 1000)
    
    return PoseData(joints=intermediate_joints, confidence=confidence, timestamp=timestamp)
