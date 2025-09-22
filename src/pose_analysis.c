/**
 * @file pose_analysis_simple.c
 * @brief 간단한 포즈 분석 구현 (ML Kit 33개 랜드마크 지원)
 * @author Exercise Segment API Team
 * @version 2.0.0
 */

#include "pose_analysis.h"
#include "math_utils.h"
#include <math.h>
#include <string.h>

// 최소 신뢰도 임계값
#define MIN_CONFIDENCE_THRESHOLD 0.5f

float calculate_segment_progress(const PoseData *current_pose,
                                 const PoseData *start_pose,
                                 const PoseData *end_pose,
                                 const JointType *care_joints,
                                 int care_joint_count) {
  if (!current_pose || !start_pose || !end_pose) {
    return 0.0f;
  }

  // 간단한 진행도 계산: 주요 관절들의 평균 거리 기반
  float total_distance = 0.0f;
  int valid_joints = 0;

  // 주요 관절들 (어깨, 골반, 무릎)
  JointType main_joints[] = {
      POSE_LANDMARK_LEFT_SHOULDER, POSE_LANDMARK_RIGHT_SHOULDER,
      POSE_LANDMARK_LEFT_HIP,      POSE_LANDMARK_RIGHT_HIP,
      POSE_LANDMARK_LEFT_KNEE,     POSE_LANDMARK_RIGHT_KNEE};

  int main_joint_count = sizeof(main_joints) / sizeof(main_joints[0]);

  for (int i = 0; i < main_joint_count; i++) {
    JointType joint = main_joints[i];

    // 신뢰도 확인
    if (current_pose->landmarks[joint].inFrameLikelihood <
            MIN_CONFIDENCE_THRESHOLD ||
        start_pose->landmarks[joint].inFrameLikelihood <
            MIN_CONFIDENCE_THRESHOLD ||
        end_pose->landmarks[joint].inFrameLikelihood <
            MIN_CONFIDENCE_THRESHOLD) {
      continue;
    }

    // 시작점에서 현재점까지의 거리
    float start_to_current =
        distance_3d(&start_pose->landmarks[joint].position,
                    &current_pose->landmarks[joint].position);

    // 시작점에서 끝점까지의 거리
    float start_to_end = distance_3d(&start_pose->landmarks[joint].position,
                                     &end_pose->landmarks[joint].position);

    if (start_to_end > 0.0f) {
      total_distance += start_to_current / start_to_end;
      valid_joints++;
    }
  }

  if (valid_joints == 0) {
    return 0.0f;
  }

  float progress = total_distance / valid_joints;
  return fmaxf(0.0f, fminf(1.0f, progress));
}

bool is_segment_completed(const PoseData *current_pose,
                          const PoseData *end_pose,
                          const JointType *care_joints, int care_joint_count,
                          float threshold) {
  if (!current_pose || !end_pose) {
    return false;
  }

  // 간단한 완료 판단: 주요 관절들의 평균 거리 기반
  float total_distance = 0.0f;
  int valid_joints = 0;

  // 주요 관절들
  JointType main_joints[] = {
      POSE_LANDMARK_LEFT_SHOULDER, POSE_LANDMARK_RIGHT_SHOULDER,
      POSE_LANDMARK_LEFT_HIP,      POSE_LANDMARK_RIGHT_HIP,
      POSE_LANDMARK_LEFT_KNEE,     POSE_LANDMARK_RIGHT_KNEE};

  int main_joint_count = sizeof(main_joints) / sizeof(main_joints[0]);

  for (int i = 0; i < main_joint_count; i++) {
    JointType joint = main_joints[i];

    // 신뢰도 확인
    if (current_pose->landmarks[joint].inFrameLikelihood <
            MIN_CONFIDENCE_THRESHOLD ||
        end_pose->landmarks[joint].inFrameLikelihood <
            MIN_CONFIDENCE_THRESHOLD) {
      continue;
    }

    // 현재점에서 목표점까지의 거리
    float distance = distance_3d(&current_pose->landmarks[joint].position,
                                 &end_pose->landmarks[joint].position);

    total_distance += distance;
    valid_joints++;
  }

  if (valid_joints == 0) {
    return false;
  }

  float avg_distance = total_distance / valid_joints;
  return avg_distance <= threshold;
}

float segment_calculate_similarity(const PoseData *current_pose,
                                   const PoseData *target_pose) {
  if (!current_pose || !target_pose) {
    return 0.0f;
  }

  // 간단한 유사도 계산: 주요 관절들의 평균 거리 기반
  float total_similarity = 0.0f;
  int valid_joints = 0;

  // 주요 관절들
  JointType main_joints[] = {
      POSE_LANDMARK_LEFT_SHOULDER, POSE_LANDMARK_RIGHT_SHOULDER,
      POSE_LANDMARK_LEFT_HIP,      POSE_LANDMARK_RIGHT_HIP,
      POSE_LANDMARK_LEFT_KNEE,     POSE_LANDMARK_RIGHT_KNEE};

  int main_joint_count = sizeof(main_joints) / sizeof(main_joints[0]);

  for (int i = 0; i < main_joint_count; i++) {
    JointType joint = main_joints[i];

    // 신뢰도 확인
    if (current_pose->landmarks[joint].inFrameLikelihood <
            MIN_CONFIDENCE_THRESHOLD ||
        target_pose->landmarks[joint].inFrameLikelihood <
            MIN_CONFIDENCE_THRESHOLD) {
      continue;
    }

    // 거리 계산
    float distance = distance_3d(&current_pose->landmarks[joint].position,
                                 &target_pose->landmarks[joint].position);

    // 거리를 유사도로 변환 (0~1 범위)
    float similarity = fmaxf(0.0f, 1.0f - (distance / 100.0f)); // 100픽셀 기준
    total_similarity += similarity;
    valid_joints++;
  }

  if (valid_joints == 0) {
    return 0.0f;
  }

  return total_similarity / valid_joints;
}

void calculate_correction_vectors(const PoseData *current_pose,
                                  const PoseData *target_pose,
                                  const JointType *care_joints,
                                  int care_joint_count,
                                  Point3D corrections[POSE_LANDMARK_COUNT]) {
  if (!current_pose || !target_pose || !corrections) {
    return;
  }

  // 모든 랜드마크에 대해 교정 벡터 계산
  for (int i = 0; i < POSE_LANDMARK_COUNT; i++) {
    // 신뢰도 확인
    if (current_pose->landmarks[i].inFrameLikelihood <
            MIN_CONFIDENCE_THRESHOLD ||
        target_pose->landmarks[i].inFrameLikelihood <
            MIN_CONFIDENCE_THRESHOLD) {
      corrections[i] = (Point3D){0.0f, 0.0f, 0.0f};
      continue;
    }

    // 교정 벡터 계산 (목표 - 현재)
    corrections[i].x = target_pose->landmarks[i].position.x -
                       current_pose->landmarks[i].position.x;
    corrections[i].y = target_pose->landmarks[i].position.y -
                       current_pose->landmarks[i].position.y;
    corrections[i].z = target_pose->landmarks[i].position.z -
                       current_pose->landmarks[i].position.z;

    // 벡터 정규화 제거 - 실제 거리 정보 유지
    // float magnitude = sqrtf(corrections[i].x * corrections[i].x +
    //                         corrections[i].y * corrections[i].y +
    //                         corrections[i].z * corrections[i].z);

    // if (magnitude > 0.0f) {
    //   corrections[i].x /= magnitude;
    //   corrections[i].y /= magnitude;
    //   corrections[i].z /= magnitude;
    // }
  }
}
