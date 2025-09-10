/**
 * @file segment_core.c
 * @brief 핵심 세그먼트 관리 함수들 구현
 * @author Exercise Segment API Team
 * @version 1.0.0
 */

#include "segment_api.h"
#include "calibration.h"
#include "pose_analysis.h"
#include "math_utils.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

// 전역 상태 변수들
static bool g_initialized = false;
static bool g_segment_loaded = false;

// API 내부 이상적 표준 포즈들
static PoseData g_ideal_base_pose;           // 이상적 기본 포즈
static PoseData g_ideal_poses[100];          // 이상적 포즈 데이터베이스
static int g_ideal_pose_count = 0;

// A 이용자용 (기록자)
static CalibrationData g_recorder_calibration;  // A의 체형 데이터
static bool g_recorder_calibrated = false;

// B 이용자용 (사용자)
static CalibrationData g_user_calibration;      // B의 체형 데이터
static bool g_user_calibrated = false;
static PoseData g_user_segment_start;          // B용 변환된 시작 포즈
static PoseData g_user_segment_end;            // B용 변환된 종료 포즈

// 에러 메시지 배열
static const char* error_messages[] = {
    "Success",
    "System not initialized",
    "Invalid pose data",
    "Calibration failed",
    "Segment not created",
    "Invalid parameter",
    "Memory allocation failed"
};

// 이상적 기본 포즈 초기화 함수
static void initialize_ideal_base_pose(void) {
    // 표준 체형의 이상적 기본 포즈 설정 (어깨 너비 40cm, 키 170cm 기준)
    g_ideal_base_pose.joints[JOINT_NOSE] = (Point3D){0.0f, -10.0f, 0.0f};
    g_ideal_base_pose.joints[JOINT_LEFT_SHOULDER] = (Point3D){-20.0f, 0.0f, 0.0f};
    g_ideal_base_pose.joints[JOINT_RIGHT_SHOULDER] = (Point3D){20.0f, 0.0f, 0.0f};
    g_ideal_base_pose.joints[JOINT_LEFT_ELBOW] = (Point3D){-30.0f, 20.0f, 0.0f};
    g_ideal_base_pose.joints[JOINT_RIGHT_ELBOW] = (Point3D){30.0f, 20.0f, 0.0f};
    g_ideal_base_pose.joints[JOINT_LEFT_WRIST] = (Point3D){-40.0f, 40.0f, 0.0f};
    g_ideal_base_pose.joints[JOINT_RIGHT_WRIST] = (Point3D){40.0f, 40.0f, 0.0f};
    g_ideal_base_pose.joints[JOINT_LEFT_HIP] = (Point3D){-10.0f, 50.0f, 0.0f};
    g_ideal_base_pose.joints[JOINT_RIGHT_HIP] = (Point3D){10.0f, 50.0f, 0.0f};
    g_ideal_base_pose.joints[JOINT_LEFT_KNEE] = (Point3D){-10.0f, 80.0f, 0.0f};
    g_ideal_base_pose.joints[JOINT_RIGHT_KNEE] = (Point3D){10.0f, 80.0f, 0.0f};
    g_ideal_base_pose.joints[JOINT_LEFT_ANKLE] = (Point3D){-10.0f, 110.0f, 0.0f};
    g_ideal_base_pose.joints[JOINT_RIGHT_ANKLE] = (Point3D){10.0f, 110.0f, 0.0f};
    
    // 모든 관절의 신뢰도를 높게 설정
    for (int i = 0; i < JOINT_COUNT; i++) {
        g_ideal_base_pose.confidence[i] = 0.9f;
    }
    
    g_ideal_base_pose.timestamp = 1000;
}

// JSON 파일 처리 함수들 (구현 필요)
static int save_pose_to_json(const PoseData* pose, const char* pose_name, const char* json_file_path);
static int finalize_json_workout(const char* workout_name, const char* json_file_path);
static int load_poses_from_json(const char* json_file_path, int start_index, int end_index, 
                                PoseData* start_pose, PoseData* end_pose);

int segment_api_init(void) {
    if (g_initialized) {
        return SEGMENT_OK;  // 이미 초기화됨
    }
    
    // 전역 상태 초기화
    memset(&g_ideal_base_pose, 0, sizeof(PoseData));
    memset(g_ideal_poses, 0, sizeof(g_ideal_poses));
    g_ideal_pose_count = 0;
    
    memset(&g_recorder_calibration, 0, sizeof(CalibrationData));
    g_recorder_calibrated = false;
    
    memset(&g_user_calibration, 0, sizeof(CalibrationData));
    g_user_calibrated = false;
    memset(&g_user_segment_start, 0, sizeof(PoseData));
    memset(&g_user_segment_end, 0, sizeof(PoseData));
    
    g_segment_loaded = false;
    g_initialized = true;
    
    // 이상적 기본 포즈 초기화 (표준 체형)
    initialize_ideal_base_pose();
    
    return SEGMENT_OK;
}

// MARK: - A 이용자 (기록자) 함수들

int segment_calibrate_recorder(const PoseData* base_pose) {
    if (!g_initialized) {
        return SEGMENT_ERROR_NOT_INITIALIZED;
    }
    
    if (!base_pose) {
        return SEGMENT_ERROR_INVALID_PARAMETER;
    }
    
    // 포즈 유효성 검사
    if (!segment_validate_pose(base_pose)) {
        return SEGMENT_ERROR_INVALID_POSE;
    }
    
    // A의 포즈를 이상적 기본 포즈와 비교하여 캘리브레이션 데이터 생성
    int result = segment_calibrate_between_poses(base_pose, &g_ideal_base_pose, &g_recorder_calibration);
    if (result != SEGMENT_OK) {
        return result;
    }
    
    g_recorder_calibrated = true;
    return SEGMENT_OK;
}

int segment_record_pose(const PoseData* current_pose, const char* pose_name, const char* json_file_path) {
    if (!g_initialized) {
        return SEGMENT_ERROR_NOT_INITIALIZED;
    }
    
    if (!g_recorder_calibrated) {
        return SEGMENT_ERROR_CALIBRATION_FAILED;
    }
    
    if (!current_pose || !pose_name || !json_file_path) {
        return SEGMENT_ERROR_INVALID_PARAMETER;
    }
    
    // A의 포즈를 이상적 비율로 변환
    PoseData ideal_pose;
    int result = apply_calibration_to_pose(current_pose, &g_recorder_calibration, &ideal_pose);
    if (result != SEGMENT_OK) {
        return result;
    }
    
    // JSON 파일에 저장 (구현 필요)
    result = save_pose_to_json(&ideal_pose, pose_name, json_file_path);
    if (result != SEGMENT_OK) {
        return result;
    }
    
    return SEGMENT_OK;
}

int segment_finalize_workout_json(const char* workout_name, const char* json_file_path) {
    if (!g_initialized) {
        return SEGMENT_ERROR_NOT_INITIALIZED;
    }
    
    if (!workout_name || !json_file_path) {
        return SEGMENT_ERROR_INVALID_PARAMETER;
    }
    
    // JSON 파일 완성 (구현 필요)
    int result = finalize_json_workout(workout_name, json_file_path);
    if (result != SEGMENT_OK) {
        return result;
    }
    
    return SEGMENT_OK;
}

// MARK: - B 이용자 (사용자) 함수들

int segment_calibrate_user(const PoseData* base_pose) {
    if (!g_initialized) {
        return SEGMENT_ERROR_NOT_INITIALIZED;
    }
    
    if (!base_pose) {
        return SEGMENT_ERROR_INVALID_PARAMETER;
    }
    
    // 포즈 유효성 검사
    if (!segment_validate_pose(base_pose)) {
        return SEGMENT_ERROR_INVALID_POSE;
    }
    
    // B의 포즈를 이상적 기본 포즈와 비교하여 캘리브레이션 데이터 생성
    int result = segment_calibrate_between_poses(base_pose, &g_ideal_base_pose, &g_user_calibration);
    if (result != SEGMENT_OK) {
        return result;
    }
    
    g_user_calibrated = true;
    return SEGMENT_OK;
}

int segment_load_segment(const char* json_file_path, int start_index, int end_index) {
    if (!g_initialized) {
        return SEGMENT_ERROR_NOT_INITIALIZED;
    }
    
    if (!g_user_calibrated) {
        return SEGMENT_ERROR_CALIBRATION_FAILED;
    }
    
    if (!json_file_path || start_index < 0 || end_index < 0) {
        return SEGMENT_ERROR_INVALID_PARAMETER;
    }
    
    // JSON 파일에서 포즈 로드 (구현 필요)
    PoseData ideal_start_pose, ideal_end_pose;
    int result = load_poses_from_json(json_file_path, start_index, end_index, &ideal_start_pose, &ideal_end_pose);
    if (result != SEGMENT_OK) {
        return result;
    }
    
    // 이상적 포즈를 B의 체형에 맞게 변환
    result = apply_calibration_to_pose(&ideal_start_pose, &g_user_calibration, &g_user_segment_start);
    if (result != SEGMENT_OK) {
        return result;
    }
    
    result = apply_calibration_to_pose(&ideal_end_pose, &g_user_calibration, &g_user_segment_end);
    if (result != SEGMENT_OK) {
        return result;
    }
    
    g_segment_loaded = true;
    return SEGMENT_OK;
}

SegmentOutput segment_analyze(const PoseData* current_pose) {
    SegmentOutput result = {0};
    
    if (!g_initialized || !g_segment_loaded || !current_pose) {
        return result;
    }
    
    // 현재 포즈와 세그먼트의 시작→종료 포즈 비교
    float progress = calculate_segment_progress(current_pose, &g_user_segment_start, &g_user_segment_end, 
                                               NULL, 0);  // 모든 관절 사용
    
    bool completed = is_segment_completed(current_pose, &g_user_segment_end, NULL, 0, 0.8f);
    
    float similarity = segment_calculate_similarity(current_pose, &g_user_segment_end);
    
    // 교정 벡터 계산
    calculate_correction_vectors(current_pose, &g_user_segment_end, NULL, 0, result.corrections);
    
    result.progress = progress;
    result.completed = completed;
    result.similarity = similarity;
    result.timestamp = current_pose->timestamp;
    
    return result;
}

int segment_get_transformed_end_pose(PoseData* out_pose) {
    if (!g_initialized || !g_segment_loaded || !out_pose) {
        return SEGMENT_ERROR_INVALID_PARAMETER;
    }
    
    *out_pose = g_user_segment_end;  // B의 체형에 맞게 변환된 종료 포즈
    return SEGMENT_OK;
}

// MARK: - JSON 파일 처리 함수들 (기본 구현)

static int save_pose_to_json(const PoseData* pose, const char* pose_name, const char* json_file_path) {
    // TODO: JSON 파일에 포즈 저장 구현
    // 현재는 기본 구현으로 SEGMENT_OK 반환
    (void)pose;
    (void)pose_name;
    (void)json_file_path;
    return SEGMENT_OK;
}

static int finalize_json_workout(const char* workout_name, const char* json_file_path) {
    // TODO: JSON 워크아웃 파일 완성 구현
    // 현재는 기본 구현으로 SEGMENT_OK 반환
    (void)workout_name;
    (void)json_file_path;
    return SEGMENT_OK;
}

static int load_poses_from_json(const char* json_file_path, int start_index, int end_index, 
                                PoseData* start_pose, PoseData* end_pose) {
    // TODO: JSON 파일에서 포즈 로드 구현
    // 현재는 기본 구현으로 더미 데이터 반환
    (void)json_file_path;
    (void)start_index;
    (void)end_index;
    
    // 더미 데이터 생성
    *start_pose = g_ideal_base_pose;
    *end_pose = g_ideal_base_pose;
    
    return SEGMENT_OK;
}

void segment_api_cleanup(void) {
    if (!g_initialized) return;
    
    // 세그먼트 해제
    segment_destroy();
    
    g_initialized = false;
}

// segment_calibrate와 segment_validate_calibration은 calibration.c에서 구현됨

int segment_reset(void) {
    if (!g_initialized || !g_segment_loaded) {
        return SEGMENT_ERROR_NOT_INITIALIZED;
    }
    
    // 세그먼트 설정은 유지하고 진행 상태만 초기화
    // 현재 구현에서는 별도의 진행 상태가 없으므로 성공 반환
    return SEGMENT_OK;
}

void segment_destroy(void) {
    g_segment_loaded = false;
    
    // 메모리 초기화
    memset(&g_user_segment_start, 0, sizeof(PoseData));
    memset(&g_user_segment_end, 0, sizeof(PoseData));
}

// segment_calculate_similarity와 segment_validate_pose는 pose_analysis.c에서 구현됨

const char* segment_get_error_message(int error_code) {
    int index = -error_code;  // 에러 코드는 음수이므로 양수로 변환
    
    if (index >= 0 && index < (int)(sizeof(error_messages) / sizeof(error_messages[0]))) {
        return error_messages[index];
    }
    
    return "Unknown error";
}

// MARK: - Swift 친화적인 함수들 구현

int segment_analyze_simple(const PoseData* current_pose,
                          float* out_progress,
                          bool* out_is_complete,
                          float* out_similarity,
                          Point3D* out_corrections) {
    // 입력 유효성 검사
    if (!g_initialized || !g_segment_loaded) {
        return SEGMENT_ERROR_NOT_INITIALIZED;
    }
    
    if (!current_pose || !out_progress || !out_is_complete || !out_similarity || !out_corrections) {
        return SEGMENT_ERROR_INVALID_PARAMETER;
    }
    
    // 새로운 segment_analyze 함수 호출
    SegmentOutput output = segment_analyze(current_pose);
    
    // 결과를 개별 변수로 복사
    *out_progress = output.progress;
    *out_is_complete = output.completed;
    *out_similarity = output.similarity;
    
    // 교정 벡터 복사
    for (int i = 0; i < JOINT_COUNT; i++) {
        out_corrections[i] = output.corrections[i];
    }
    
    return SEGMENT_OK;
}

int segment_create_pose_data(const Point3D* joints, 
                            float confidence, 
                            PoseData* out_pose) {
    if (!joints || !out_pose) {
        return SEGMENT_ERROR_INVALID_PARAMETER;
    }
    
    // 13개 관절 복사 (배열로 복사)
    for (int i = 0; i < JOINT_COUNT; i++) {
        out_pose->joints[i] = joints[i];
        out_pose->confidence[i] = confidence;
    }
    
    out_pose->timestamp = 0; // 기본값
    
    return SEGMENT_OK;
}

