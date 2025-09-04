/**
 * @file pose_analysis.c
 * @brief 포즈 분석 및 진행도 계산 함수들 구현
 * @author Exercise Segment API Team
 * @version 1.0.0
 */

#include "pose_analysis.h"
#include "math_utils.h"
#include <math.h>
#include <string.h>

#define MIN_CONFIDENCE_THRESHOLD 0.7f
#define DEFAULT_SIMILARITY_THRESHOLD 0.8f
#define MAX_POSSIBLE_DISTANCE 200.0f  // 최대 가능한 관절 간 거리 (cm)

float segment_calculate_similarity(const PoseData* pose1, const PoseData* pose2) {
    if (!pose1 || !pose2) return 0.0f;
    
    float total_distance = 0.0f;
    int valid_joints = 0;
    
    for (int i = 0; i < JOINT_COUNT; i++) {
        // 두 포즈 모두에서 신뢰도가 충분한 관절만 사용
        if (pose1->confidence[i] > MIN_CONFIDENCE_THRESHOLD && 
            pose2->confidence[i] > MIN_CONFIDENCE_THRESHOLD) {
            
            float distance = distance_3d(&pose1->joints[i], &pose2->joints[i]);
            total_distance += distance;
            valid_joints++;
        }
    }
    
    if (valid_joints == 0) return 0.0f;
    
    float average_distance = total_distance / valid_joints;
    float similarity = 1.0f - (average_distance / MAX_POSSIBLE_DISTANCE);
    
    return clamp(similarity, 0.0f, 1.0f);
}

bool segment_validate_pose(const PoseData* pose) {
    if (!pose) return false;
    
    // 필수 관절들의 신뢰도 확인
    int essential_joints[] = {
        JOINT_LEFT_SHOULDER, JOINT_RIGHT_SHOULDER,
        JOINT_LEFT_HIP, JOINT_RIGHT_HIP
    };
    
    int essential_count = sizeof(essential_joints) / sizeof(essential_joints[0]);
    int valid_essential = 0;
    
    for (int i = 0; i < essential_count; i++) {
        JointType joint = essential_joints[i];
        if (pose->confidence[joint] > MIN_CONFIDENCE_THRESHOLD) {
            valid_essential++;
        }
    }
    
    // 필수 관절 중 최소 3개는 유효해야 함
    if (valid_essential < 3) return false;
    
    // 좌표값이 합리적 범위 내인지 확인
    for (int i = 0; i < JOINT_COUNT; i++) {
        if (pose->confidence[i] > 0.0f) {
            const Point3D* joint = &pose->joints[i];
            if (fabsf(joint->x) > 1000.0f || 
                fabsf(joint->y) > 1000.0f || 
                fabsf(joint->z) > 1000.0f) {
                return false;
            }
        }
    }
    
    // 타임스탬프 유효성 (0이면 무효)
    if (pose->timestamp == 0) return false;
    
    return true;
}

float calculate_segment_progress(const PoseData* current_pose,
                                const PoseData* start_pose,
                                const PoseData* end_pose,
                                const JointType* care_joints,
                                int care_joint_count) {
    if (!current_pose || !start_pose || !end_pose || !care_joints || care_joint_count <= 0) {
        return 0.0f;
    }
    
    float total_movement = 0.0f;
    float current_movement = 0.0f;
    
    for (int i = 0; i < care_joint_count; i++) {
        JointType joint = care_joints[i];
        
        if (joint < 0 || joint >= JOINT_COUNT) continue;
        
        // 세 포즈 모두에서 신뢰도가 충분한 관절만 사용
        if (current_pose->confidence[joint] > MIN_CONFIDENCE_THRESHOLD &&
            start_pose->confidence[joint] > MIN_CONFIDENCE_THRESHOLD &&
            end_pose->confidence[joint] > MIN_CONFIDENCE_THRESHOLD) {
            
            // 시작에서 끝까지의 전체 움직임 거리
            float full_movement = distance_3d(&start_pose->joints[joint], 
                                            &end_pose->joints[joint]);
            
            // 시작에서 현재까지의 움직임 거리
            float current_distance = distance_3d(&start_pose->joints[joint], 
                                               &current_pose->joints[joint]);
            
            total_movement += full_movement;
            current_movement += current_distance;
        }
    }
    
    if (total_movement <= 0.0f) return 0.0f;
    
    float progress = current_movement / total_movement;
    return clamp(progress, 0.0f, 1.0f);
}

void calculate_correction_vectors(const PoseData* current_pose,
                                 const PoseData* target_pose,
                                 const JointType* care_joints,
                                 int care_joint_count,
                                 Point3D corrections[JOINT_COUNT]) {
    if (!current_pose || !target_pose || !care_joints || !corrections) return;
    
    // 모든 교정 벡터를 0으로 초기화
    for (int i = 0; i < JOINT_COUNT; i++) {
        corrections[i].x = 0.0f;
        corrections[i].y = 0.0f;
        corrections[i].z = 0.0f;
    }
    
    // 관심 관절들에 대해서만 교정 벡터 계산
    for (int i = 0; i < care_joint_count; i++) {
        JointType joint = care_joints[i];
        
        if (joint < 0 || joint >= JOINT_COUNT) continue;
        
        // 두 포즈 모두에서 신뢰도가 충분한 관절만 처리
        if (current_pose->confidence[joint] > MIN_CONFIDENCE_THRESHOLD &&
            target_pose->confidence[joint] > MIN_CONFIDENCE_THRESHOLD) {
            
            // 목표 위치에서 현재 위치를 뺀 벡터 (교정 방향)
            corrections[joint] = subtract_points(&target_pose->joints[joint], 
                                               &current_pose->joints[joint]);
        }
    }
}

bool is_segment_completed(const PoseData* current_pose,
                         const PoseData* end_pose,
                         const JointType* care_joints,
                         int care_joint_count,
                         float similarity_threshold) {
    if (!current_pose || !end_pose || !care_joints || care_joint_count <= 0) {
        return false;
    }
    
    // 기본 임계값 사용
    if (similarity_threshold <= 0.0f) {
        similarity_threshold = DEFAULT_SIMILARITY_THRESHOLD;
    }
    
    // 관심 관절들만으로 유사도 계산
    float total_distance = 0.0f;
    int valid_joints = 0;
    
    for (int i = 0; i < care_joint_count; i++) {
        JointType joint = care_joints[i];
        
        if (joint < 0 || joint >= JOINT_COUNT) continue;
        
        if (current_pose->confidence[joint] > MIN_CONFIDENCE_THRESHOLD &&
            end_pose->confidence[joint] > MIN_CONFIDENCE_THRESHOLD) {
            
            float distance = distance_3d(&current_pose->joints[joint], 
                                       &end_pose->joints[joint]);
            total_distance += distance;
            valid_joints++;
        }
    }
    
    if (valid_joints == 0) return false;
    
    float average_distance = total_distance / valid_joints;
    float similarity = 1.0f - (average_distance / MAX_POSSIBLE_DISTANCE);
    
    return similarity >= similarity_threshold;
}

void normalize_pose_center(const PoseData* input_pose,
                          const Point3D* reference_center,
                          PoseData* normalized_pose) {
    if (!input_pose || !reference_center || !normalized_pose) return;
    
    // 입력 포즈의 중심점 계산
    Point3D input_center = calculate_center_point(input_pose);
    
    // 중심점 차이 계산
    Point3D offset = subtract_points(reference_center, &input_center);
    
    // 모든 관절을 오프셋만큼 이동
    translate_pose(input_pose, &offset, normalized_pose);
}
