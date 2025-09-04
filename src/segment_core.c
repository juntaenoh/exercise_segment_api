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
static bool g_segment_created = false;

// 현재 세그먼트 데이터
static PoseData g_start_keypose;
static PoseData g_end_keypose;
static PoseData g_personalized_start;
static PoseData g_personalized_end;
static CalibrationData g_calibration;
static JointType* g_care_joints = NULL;
static int g_care_joint_count = 0;

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

int segment_api_init(void) {
    if (g_initialized) {
        return SEGMENT_OK;  // 이미 초기화됨
    }
    
    // 전역 상태 초기화
    memset(&g_start_keypose, 0, sizeof(PoseData));
    memset(&g_end_keypose, 0, sizeof(PoseData));
    memset(&g_personalized_start, 0, sizeof(PoseData));
    memset(&g_personalized_end, 0, sizeof(PoseData));
    memset(&g_calibration, 0, sizeof(CalibrationData));
    
    g_care_joints = NULL;
    g_care_joint_count = 0;
    g_segment_created = false;
    g_initialized = true;
    
    return SEGMENT_OK;
}

void segment_api_cleanup(void) {
    if (!g_initialized) return;
    
    // 세그먼트 해제
    segment_destroy();
    
    // 메모리 해제
    if (g_care_joints) {
        free(g_care_joints);
        g_care_joints = NULL;
    }
    
    g_initialized = false;
}

// segment_calibrate와 segment_validate_calibration은 calibration.c에서 구현됨

int segment_create(const PoseData* start_keypose, 
                   const PoseData* end_keypose,
                   const CalibrationData* calibration,
                   const JointType* care_joints,
                   int care_joint_count) {
    if (!g_initialized) {
        return SEGMENT_ERROR_NOT_INITIALIZED;
    }
    
    if (!start_keypose || !end_keypose || !calibration || !care_joints || care_joint_count <= 0) {
        return SEGMENT_ERROR_INVALID_PARAMETER;
    }
    
    // 입력 데이터 유효성 검사
    if (!segment_validate_pose(start_keypose) || !segment_validate_pose(end_keypose)) {
        return SEGMENT_ERROR_INVALID_POSE;
    }
    
    if (!segment_validate_calibration(calibration)) {
        return SEGMENT_ERROR_CALIBRATION_FAILED;
    }
    
    // 기존 세그먼트가 있다면 해제
    segment_destroy();
    
    // 키포즈 복사
    memcpy(&g_start_keypose, start_keypose, sizeof(PoseData));
    memcpy(&g_end_keypose, end_keypose, sizeof(PoseData));
    memcpy(&g_calibration, calibration, sizeof(CalibrationData));
    
    // 관심 관절 배열 할당 및 복사
    g_care_joints = (JointType*)malloc(care_joint_count * sizeof(JointType));
    if (!g_care_joints) {
        return SEGMENT_ERROR_MEMORY_ALLOCATION;
    }
    
    memcpy(g_care_joints, care_joints, care_joint_count * sizeof(JointType));
    g_care_joint_count = care_joint_count;
    
    // 키포즈들을 캘리브레이션에 따라 개인화
    int result = apply_calibration_to_pose(&g_start_keypose, &g_calibration, &g_personalized_start);
    if (result != SEGMENT_OK) {
        segment_destroy();
        return result;
    }
    
    result = apply_calibration_to_pose(&g_end_keypose, &g_calibration, &g_personalized_end);
    if (result != SEGMENT_OK) {
        segment_destroy();
        return result;
    }
    
    g_segment_created = true;
    return SEGMENT_OK;
}

SegmentOutput segment_analyze(const SegmentInput* input) {
    SegmentOutput output = {0};
    
    if (!g_initialized || !g_segment_created || !input) {
        return output;  // 기본값 반환
    }
    
    // 입력 포즈 유효성 검사
    if (!segment_validate_pose(&input->raw_pose)) {
        return output;
    }
    
    // 현재 포즈를 캘리브레이션에 따라 정규화
    PoseData calibrated_pose;
    int result = apply_calibration_to_pose(&input->raw_pose, &g_calibration, &calibrated_pose);
    if (result != SEGMENT_OK) {
        return output;
    }
    
    // 정규화된 포즈의 중심점을 개인화된 키포즈 중심점에 맞춤
    Point3D personalized_center = calculate_center_point(&g_personalized_start);
    PoseData normalized_pose;
    normalize_pose_center(&calibrated_pose, &personalized_center, &normalized_pose);
    
    // 진행도 계산
    float progress = calculate_segment_progress(&normalized_pose, 
                                              &g_personalized_start, 
                                              &g_personalized_end,
                                              g_care_joints, 
                                              g_care_joint_count);
    
    // 목표 키포즈 계산 (진행도에 따른 보간)
    PoseData target_pose;
    interpolate_pose(&g_personalized_start, &g_personalized_end, progress, &target_pose);
    
    // 각 관절별 교정 벡터 계산
    calculate_correction_vectors(&normalized_pose, 
                                &target_pose, 
                                g_care_joints, 
                                g_care_joint_count, 
                                output.corrections);
    
    // 완료 여부 판단
    bool completed = is_segment_completed(&normalized_pose, 
                                        &g_personalized_end, 
                                        g_care_joints, 
                                        g_care_joint_count, 
                                        0.8f);
    
    // 유사도 계산
    float similarity = segment_calculate_similarity(&normalized_pose, &target_pose);
    
    // 결과 저장
    output.progress = progress;
    output.completed = completed;
    output.similarity = similarity;
    output.timestamp = input->raw_pose.timestamp;
    
    return output;
}

int segment_reset(void) {
    if (!g_initialized || !g_segment_created) {
        return SEGMENT_ERROR_NOT_INITIALIZED;
    }
    
    // 세그먼트 설정은 유지하고 진행 상태만 초기화
    // 현재 구현에서는 별도의 진행 상태가 없으므로 성공 반환
    return SEGMENT_OK;
}

void segment_destroy(void) {
    if (g_care_joints) {
        free(g_care_joints);
        g_care_joints = NULL;
    }
    
    g_care_joint_count = 0;
    g_segment_created = false;
    
    // 메모리 초기화
    memset(&g_start_keypose, 0, sizeof(PoseData));
    memset(&g_end_keypose, 0, sizeof(PoseData));
    memset(&g_personalized_start, 0, sizeof(PoseData));
    memset(&g_personalized_end, 0, sizeof(PoseData));
    memset(&g_calibration, 0, sizeof(CalibrationData));
}

// segment_calculate_similarity와 segment_validate_pose는 pose_analysis.c에서 구현됨

const char* segment_get_error_message(int error_code) {
    int index = -error_code;  // 에러 코드는 음수이므로 양수로 변환
    
    if (index >= 0 && index < (int)(sizeof(error_messages) / sizeof(error_messages[0]))) {
        return error_messages[index];
    }
    
    return "Unknown error";
}
