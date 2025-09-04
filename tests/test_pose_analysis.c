/**
 * @file test_pose_analysis.c
 * @brief 포즈 분석 함수들 테스트
 * @author Exercise Segment API Team
 * @version 1.0.0
 */

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "pose_analysis.h"
#include "segment_types.h"

#define EPSILON 0.001f

void create_test_pose(PoseData* pose, float offset_x, float offset_y, float offset_z) {
    if (!pose) return;
    
    // 기본 관절 위치 설정
    pose->joints[JOINT_NOSE] = (Point3D){0.0f + offset_x, -10.0f + offset_y, 0.0f + offset_z};
    pose->joints[JOINT_LEFT_SHOULDER] = (Point3D){-20.0f + offset_x, 0.0f + offset_y, 0.0f + offset_z};
    pose->joints[JOINT_RIGHT_SHOULDER] = (Point3D){20.0f + offset_x, 0.0f + offset_y, 0.0f + offset_z};
    pose->joints[JOINT_LEFT_ELBOW] = (Point3D){-30.0f + offset_x, 20.0f + offset_y, 0.0f + offset_z};
    pose->joints[JOINT_RIGHT_ELBOW] = (Point3D){30.0f + offset_x, 20.0f + offset_y, 0.0f + offset_z};
    pose->joints[JOINT_LEFT_WRIST] = (Point3D){-40.0f + offset_x, 40.0f + offset_y, 0.0f + offset_z};
    pose->joints[JOINT_RIGHT_WRIST] = (Point3D){40.0f + offset_x, 40.0f + offset_y, 0.0f + offset_z};
    pose->joints[JOINT_LEFT_HIP] = (Point3D){-10.0f + offset_x, 50.0f + offset_y, 0.0f + offset_z};
    pose->joints[JOINT_RIGHT_HIP] = (Point3D){10.0f + offset_x, 50.0f + offset_y, 0.0f + offset_z};
    pose->joints[JOINT_LEFT_KNEE] = (Point3D){-10.0f + offset_x, 80.0f + offset_y, 0.0f + offset_z};
    pose->joints[JOINT_RIGHT_KNEE] = (Point3D){10.0f + offset_x, 80.0f + offset_y, 0.0f + offset_z};
    pose->joints[JOINT_LEFT_ANKLE] = (Point3D){-10.0f + offset_x, 110.0f + offset_y, 0.0f + offset_z};
    pose->joints[JOINT_RIGHT_ANKLE] = (Point3D){10.0f + offset_x, 110.0f + offset_y, 0.0f + offset_z};
    
    // 모든 관절의 신뢰도를 높게 설정
    for (int i = 0; i < JOINT_COUNT; i++) {
        pose->confidence[i] = 0.9f;
    }
    
    pose->timestamp = 1000;
}

void test_segment_calculate_similarity() {
    printf("테스트: segment_calculate_similarity\n");
    
    PoseData pose1, pose2;
    create_test_pose(&pose1, 0.0f, 0.0f, 0.0f);
    create_test_pose(&pose2, 0.0f, 0.0f, 0.0f);
    
    // 동일한 포즈
    float similarity = segment_calculate_similarity(&pose1, &pose2);
    assert(similarity > 0.9f);  // 매우 높은 유사도
    
    printf("  ✓ 동일 포즈 유사도 테스트 통과\n");
    
    // 다른 포즈 (더 큰 차이)
    create_test_pose(&pose2, 50.0f, 50.0f, 50.0f);
    similarity = segment_calculate_similarity(&pose1, &pose2);
    assert(similarity < 0.9f);  // 낮은 유사도
    assert(similarity >= 0.0f && similarity <= 1.0f);  // 범위 내
    
    printf("  ✓ 다른 포즈 유사도 테스트 통과\n");
    
    // NULL 포인터 테스트
    similarity = segment_calculate_similarity(NULL, &pose2);
    assert(similarity == 0.0f);
    
    printf("  ✓ NULL 포인터 유사도 테스트 통과\n");
}

void test_segment_validate_pose() {
    printf("테스트: segment_validate_pose\n");
    
    PoseData valid_pose;
    create_test_pose(&valid_pose, 0.0f, 0.0f, 0.0f);
    
    assert(segment_validate_pose(&valid_pose) == true);
    printf("  ✓ 유효한 포즈 검증 테스트 통과\n");
    
    // 신뢰도가 낮은 포즈
    PoseData low_confidence_pose;
    create_test_pose(&low_confidence_pose, 0.0f, 0.0f, 0.0f);
    for (int i = 0; i < JOINT_COUNT; i++) {
        low_confidence_pose.confidence[i] = 0.3f;  // 낮은 신뢰도
    }
    
    assert(segment_validate_pose(&low_confidence_pose) == false);
    printf("  ✓ 낮은 신뢰도 포즈 검증 테스트 통과\n");
    
    // 타임스탬프가 0인 포즈
    PoseData invalid_timestamp_pose;
    create_test_pose(&invalid_timestamp_pose, 0.0f, 0.0f, 0.0f);
    invalid_timestamp_pose.timestamp = 0;
    
    assert(segment_validate_pose(&invalid_timestamp_pose) == false);
    printf("  ✓ 잘못된 타임스탬프 포즈 검증 테스트 통과\n");
    
    // NULL 포인터 테스트
    assert(segment_validate_pose(NULL) == false);
    printf("  ✓ NULL 포인터 포즈 검증 테스트 통과\n");
}

void test_calculate_segment_progress() {
    printf("테스트: calculate_segment_progress\n");
    
    PoseData start_pose, end_pose, current_pose;
    create_test_pose(&start_pose, 0.0f, 0.0f, 0.0f);
    create_test_pose(&end_pose, 0.0f, 0.0f, 0.0f);
    
    // 종료 포즈를 다르게 설정 (무릎을 구부린 상태)
    end_pose.joints[JOINT_LEFT_KNEE].y += 30.0f;
    end_pose.joints[JOINT_RIGHT_KNEE].y += 30.0f;
    end_pose.joints[JOINT_LEFT_HIP].y += 20.0f;
    end_pose.joints[JOINT_RIGHT_HIP].y += 20.0f;
    
    JointType care_joints[] = {JOINT_LEFT_KNEE, JOINT_RIGHT_KNEE, JOINT_LEFT_HIP, JOINT_RIGHT_HIP};
    
    // 시작 지점 (진행도 0.0)
    create_test_pose(&current_pose, 0.0f, 0.0f, 0.0f);
    float progress = calculate_segment_progress(&current_pose, &start_pose, &end_pose, 
                                               care_joints, 4);
    assert(fabsf(progress - 0.0f) < EPSILON);
    
    printf("  ✓ 시작 지점 진행도 테스트 통과\n");
    
    // 중간 지점 (진행도 0.5)
    create_test_pose(&current_pose, 0.0f, 0.0f, 0.0f);
    current_pose.joints[JOINT_LEFT_KNEE].y += 15.0f;
    current_pose.joints[JOINT_RIGHT_KNEE].y += 15.0f;
    current_pose.joints[JOINT_LEFT_HIP].y += 10.0f;
    current_pose.joints[JOINT_RIGHT_HIP].y += 10.0f;
    
    progress = calculate_segment_progress(&current_pose, &start_pose, &end_pose, 
                                        care_joints, 4);
    assert(fabsf(progress - 0.5f) < 0.1f);  // 약 0.5
    
    printf("  ✓ 중간 지점 진행도 테스트 통과\n");
    
    // 종료 지점 (진행도 1.0)
    create_test_pose(&current_pose, 0.0f, 0.0f, 0.0f);
    current_pose.joints[JOINT_LEFT_KNEE].y += 30.0f;
    current_pose.joints[JOINT_RIGHT_KNEE].y += 30.0f;
    current_pose.joints[JOINT_LEFT_HIP].y += 20.0f;
    current_pose.joints[JOINT_RIGHT_HIP].y += 20.0f;
    
    progress = calculate_segment_progress(&current_pose, &start_pose, &end_pose, 
                                        care_joints, 4);
    assert(fabsf(progress - 1.0f) < 0.1f);  // 약 1.0
    
    printf("  ✓ 종료 지점 진행도 테스트 통과\n");
}

void test_calculate_correction_vectors() {
    printf("테스트: calculate_correction_vectors\n");
    
    PoseData current_pose, target_pose;
    create_test_pose(&current_pose, 0.0f, 0.0f, 0.0f);
    create_test_pose(&target_pose, 5.0f, 5.0f, 5.0f);  // 5만큼 이동된 목표
    
    JointType care_joints[] = {JOINT_LEFT_KNEE, JOINT_RIGHT_KNEE};
    Point3D corrections[JOINT_COUNT];
    
    calculate_correction_vectors(&current_pose, &target_pose, care_joints, 2, corrections);
    
    // 관심 관절들의 교정 벡터가 올바른지 확인
    Point3D left_knee_correction = corrections[JOINT_LEFT_KNEE];
    assert(fabsf(left_knee_correction.x - 5.0f) < EPSILON);
    assert(fabsf(left_knee_correction.y - 5.0f) < EPSILON);
    assert(fabsf(left_knee_correction.z - 5.0f) < EPSILON);
    
    // 관심이 아닌 관절의 교정 벡터는 0이어야 함
    Point3D nose_correction = corrections[JOINT_NOSE];
    assert(fabsf(nose_correction.x) < EPSILON);
    assert(fabsf(nose_correction.y) < EPSILON);
    assert(fabsf(nose_correction.z) < EPSILON);
    
    printf("  ✓ 교정 벡터 계산 테스트 통과\n");
}

void test_is_segment_completed() {
    printf("테스트: is_segment_completed\n");
    
    PoseData current_pose, end_pose;
    create_test_pose(&current_pose, 0.0f, 0.0f, 0.0f);
    create_test_pose(&end_pose, 0.0f, 0.0f, 0.0f);
    
    JointType care_joints[] = {JOINT_LEFT_KNEE, JOINT_RIGHT_KNEE};
    
    // 동일한 포즈 (완료됨)
    bool completed = is_segment_completed(&current_pose, &end_pose, care_joints, 2, 0.8f);
    assert(completed == true);
    
    printf("  ✓ 동일 포즈 완료 테스트 통과\n");
    
    // 다른 포즈 (미완료) - 더 큰 차이
    create_test_pose(&current_pose, 50.0f, 50.0f, 50.0f);
    completed = is_segment_completed(&current_pose, &end_pose, care_joints, 2, 0.8f);
    assert(completed == false);
    
    printf("  ✓ 다른 포즈 미완료 테스트 통과\n");
}

void test_normalize_pose_center() {
    printf("테스트: normalize_pose_center\n");
    
    PoseData input_pose, normalized_pose;
    create_test_pose(&input_pose, 10.0f, 20.0f, 30.0f);
    
    Point3D reference_center = {0.0f, 0.0f, 0.0f};
    normalize_pose_center(&input_pose, &reference_center, &normalized_pose);
    
    // 정규화된 포즈의 중심점이 참조 중심점과 일치하는지 확인
    Point3D normalized_center = {0.0f, 0.0f, 0.0f};  // 간단한 중심점 계산
    for (int i = 0; i < JOINT_COUNT; i++) {
        normalized_center.x += normalized_pose.joints[i].x;
        normalized_center.y += normalized_pose.joints[i].y;
        normalized_center.z += normalized_pose.joints[i].z;
    }
    normalized_center.x /= JOINT_COUNT;
    normalized_center.y /= JOINT_COUNT;
    normalized_center.z /= JOINT_COUNT;
    
    assert(fabsf(normalized_center.x - reference_center.x) < 1.0f);
    assert(fabsf(normalized_center.y - reference_center.y) < 1.0f);
    assert(fabsf(normalized_center.z - reference_center.z) < 1.0f);
    
    printf("  ✓ 포즈 중심점 정규화 테스트 통과\n");
}

void test_pose_analysis_error_cases() {
    printf("테스트: 포즈 분석 에러 케이스\n");
    
    PoseData pose1, pose2;
    create_test_pose(&pose1, 0.0f, 0.0f, 0.0f);
    create_test_pose(&pose2, 0.0f, 0.0f, 0.0f);
    
    JointType care_joints[] = {JOINT_LEFT_KNEE, JOINT_RIGHT_KNEE};
    Point3D corrections[JOINT_COUNT];
    
    // NULL 포인터 테스트들
    float progress = calculate_segment_progress(NULL, &pose1, &pose2, care_joints, 2);
    assert(fabsf(progress - 0.0f) < EPSILON);
    
    calculate_correction_vectors(NULL, &pose2, care_joints, 2, corrections);
    // 에러가 발생하지 않아야 함
    
    bool completed = is_segment_completed(NULL, &pose2, care_joints, 2, 0.8f);
    assert(completed == false);
    
    printf("  ✓ NULL 포인터 에러 케이스 테스트 통과\n");
}

int main() {
    printf("=== 포즈 분석 함수 테스트 ===\n\n");
    
    test_segment_calculate_similarity();
    test_segment_validate_pose();
    test_calculate_segment_progress();
    test_calculate_correction_vectors();
    test_is_segment_completed();
    test_normalize_pose_center();
    test_pose_analysis_error_cases();
    
    printf("\n=== 모든 테스트 통과! ===\n");
    return 0;
}
