#include "include/segment_api.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// 발 중심점 계산 함수
Point3D calculate_foot_center(const PoseData *pose) {
  PoseLandmark left_ankle = pose->landmarks[POSE_LANDMARK_LEFT_ANKLE];
  PoseLandmark right_ankle = pose->landmarks[POSE_LANDMARK_RIGHT_ANKLE];

  Point3D foot_center = {0};
  if (left_ankle.inFrameLikelihood >= 0.3f &&
      right_ankle.inFrameLikelihood >= 0.3f) {
    foot_center.x = (left_ankle.position.x + right_ankle.position.x) / 2.0f;
    foot_center.y = (left_ankle.position.y + right_ankle.position.y) / 2.0f;
    foot_center.z = (left_ankle.position.z + right_ankle.position.z) / 2.0f;
  } else if (left_ankle.inFrameLikelihood >= 0.3f) {
    foot_center = left_ankle.position;
  } else if (right_ankle.inFrameLikelihood >= 0.3f) {
    foot_center = right_ankle.position;
  }
  return foot_center;
}

// 어깨 너비 계산 함수
float calculate_shoulder_width(const PoseData *pose) {
  PoseLandmark left_shoulder = pose->landmarks[POSE_LANDMARK_LEFT_SHOULDER];
  PoseLandmark right_shoulder = pose->landmarks[POSE_LANDMARK_RIGHT_SHOULDER];

  if (left_shoulder.inFrameLikelihood >= 0.5f &&
      right_shoulder.inFrameLikelihood >= 0.5f) {
    float dx = left_shoulder.position.x - right_shoulder.position.x;
    float dy = left_shoulder.position.y - right_shoulder.position.y;
    return sqrtf(dx * dx + dy * dy);
  }
  return 0;
}

int main() {
  printf("=== 발 중심점 정렬 테스트 ===\n\n");

  // 1. API 초기화
  printf("1️⃣ API 초기화\n");
  if (segment_api_init() != SEGMENT_OK) {
    printf("❌ API 초기화 실패\n");
    return -1;
  }
  printf("✅ API 초기화 완료\n\n");

  // 2. 캘리브레이션 포즈 생성 (기준 포즈)
  printf("2️⃣ 캘리브레이션 포즈 생성\n");
  PoseData calibration_pose = {0};
  calibration_pose.timestamp = 1000;

  // 기준 포즈 설정 (정면, 팔 벌린 자세)
  calibration_pose.landmarks[POSE_LANDMARK_NOSE].position =
      (Point3D){400, 200, 0};
  calibration_pose.landmarks[POSE_LANDMARK_NOSE].inFrameLikelihood = 0.9f;

  // 어깨 (너비 200)
  calibration_pose.landmarks[POSE_LANDMARK_LEFT_SHOULDER].position =
      (Point3D){300, 300, 0};
  calibration_pose.landmarks[POSE_LANDMARK_LEFT_SHOULDER].inFrameLikelihood =
      0.9f;
  calibration_pose.landmarks[POSE_LANDMARK_RIGHT_SHOULDER].position =
      (Point3D){500, 300, 0};
  calibration_pose.landmarks[POSE_LANDMARK_RIGHT_SHOULDER].inFrameLikelihood =
      0.9f;

  // 엉덩이
  calibration_pose.landmarks[POSE_LANDMARK_LEFT_HIP].position =
      (Point3D){350, 500, 0};
  calibration_pose.landmarks[POSE_LANDMARK_LEFT_HIP].inFrameLikelihood = 0.9f;
  calibration_pose.landmarks[POSE_LANDMARK_RIGHT_HIP].position =
      (Point3D){450, 500, 0};
  calibration_pose.landmarks[POSE_LANDMARK_RIGHT_HIP].inFrameLikelihood = 0.9f;

  // 발목 (중심점 400)
  calibration_pose.landmarks[POSE_LANDMARK_LEFT_ANKLE].position =
      (Point3D){350, 800, 0};
  calibration_pose.landmarks[POSE_LANDMARK_LEFT_ANKLE].inFrameLikelihood = 0.9f;
  calibration_pose.landmarks[POSE_LANDMARK_RIGHT_ANKLE].position =
      (Point3D){450, 800, 0};
  calibration_pose.landmarks[POSE_LANDMARK_RIGHT_ANKLE].inFrameLikelihood =
      0.9f;

  Point3D calib_foot_center = calculate_foot_center(&calibration_pose);
  float calib_shoulder_width = calculate_shoulder_width(&calibration_pose);

  printf("  캘리브레이션 포즈:\n");
  printf("    👃 코: (%.1f, %.1f)\n",
         calibration_pose.landmarks[POSE_LANDMARK_NOSE].position.x,
         calibration_pose.landmarks[POSE_LANDMARK_NOSE].position.y);
  printf("    👐 어깨 너비: %.1f\n", calib_shoulder_width);
  printf("    🦶 발 중심점: (%.1f, %.1f)\n", calib_foot_center.x,
         calib_foot_center.y);

  // 캘리브레이션 수행
  if (segment_calibrate_user(&calibration_pose) != SEGMENT_OK) {
    printf("❌ 캘리브레이션 실패\n");
    return -1;
  }
  printf("✅ 캘리브레이션 완료\n\n");

  // 3. JSON 로드 및 세그먼트 설정
  printf("3️⃣ JSON 로드 및 세그먼트 설정\n");
  if (segment_load_all_segments("build/test_workout.json") != SEGMENT_OK) {
    printf("❌ 세그먼트 로드 실패\n");
    return -1;
  }

  if (segment_set_current_segment(0, 0) != SEGMENT_OK) {
    printf("❌ 세그먼트 설정 실패\n");
    return -1;
  }
  printf("✅ 세그먼트 설정 완료\n\n");

  // 4. 실제 사용자 포즈 생성 (다른 위치, 다른 크기)
  printf("4️⃣ 실제 사용자 포즈 생성\n");
  PoseData current_pose = {0};
  current_pose.timestamp = 2000;

  // 실제 포즈 (오른쪽으로 이동, 크기 작게)
  current_pose.landmarks[POSE_LANDMARK_NOSE].position = (Point3D){600, 150, 0};
  current_pose.landmarks[POSE_LANDMARK_NOSE].inFrameLikelihood = 0.9f;

  // 어깨 (너비 120)
  current_pose.landmarks[POSE_LANDMARK_LEFT_SHOULDER].position =
      (Point3D){540, 250, 0};
  current_pose.landmarks[POSE_LANDMARK_LEFT_SHOULDER].inFrameLikelihood = 0.9f;
  current_pose.landmarks[POSE_LANDMARK_RIGHT_SHOULDER].position =
      (Point3D){660, 250, 0};
  current_pose.landmarks[POSE_LANDMARK_RIGHT_SHOULDER].inFrameLikelihood = 0.9f;

  // 엉덩이
  current_pose.landmarks[POSE_LANDMARK_LEFT_HIP].position =
      (Point3D){570, 450, 0};
  current_pose.landmarks[POSE_LANDMARK_LEFT_HIP].inFrameLikelihood = 0.9f;
  current_pose.landmarks[POSE_LANDMARK_RIGHT_HIP].position =
      (Point3D){630, 450, 0};
  current_pose.landmarks[POSE_LANDMARK_RIGHT_HIP].inFrameLikelihood = 0.9f;

  // 발목 (중심점 600)
  current_pose.landmarks[POSE_LANDMARK_LEFT_ANKLE].position =
      (Point3D){570, 650, 0};
  current_pose.landmarks[POSE_LANDMARK_LEFT_ANKLE].inFrameLikelihood = 0.9f;
  current_pose.landmarks[POSE_LANDMARK_RIGHT_ANKLE].position =
      (Point3D){630, 650, 0};
  current_pose.landmarks[POSE_LANDMARK_RIGHT_ANKLE].inFrameLikelihood = 0.9f;

  Point3D current_foot_center = calculate_foot_center(&current_pose);
  float current_shoulder_width = calculate_shoulder_width(&current_pose);

  printf("  현재 사용자 포즈:\n");
  printf("    👃 코: (%.1f, %.1f)\n",
         current_pose.landmarks[POSE_LANDMARK_NOSE].position.x,
         current_pose.landmarks[POSE_LANDMARK_NOSE].position.y);
  printf("    👐 어깨 너비: %.1f\n", current_shoulder_width);
  printf("    🦶 발 중심점: (%.1f, %.1f)\n", current_foot_center.x,
         current_foot_center.y);
  printf("\n");

  // 5. Smart 분석 수행
  printf("5️⃣ Smart 분석 수행\n");
  float progress, similarity;
  bool is_complete;
  Point3D corrections[POSE_LANDMARK_COUNT];
  PoseData target_pose;

  int result = segment_analyze_smart(&current_pose, &progress, &similarity,
                                     &is_complete, corrections, &target_pose);

  if (result != SEGMENT_OK) {
    printf("❌ Smart 분석 실패: %d\n", result);
    return -1;
  }

  Point3D target_foot_center = calculate_foot_center(&target_pose);
  float target_shoulder_width = calculate_shoulder_width(&target_pose);

  printf("  결과 목표 포즈:\n");
  printf("    👃 코: (%.1f, %.1f)\n",
         target_pose.landmarks[POSE_LANDMARK_NOSE].position.x,
         target_pose.landmarks[POSE_LANDMARK_NOSE].position.y);
  printf("    👐 어깨 너비: %.1f\n", target_shoulder_width);
  printf("    🦶 발 중심점: (%.1f, %.1f)\n", target_foot_center.x,
         target_foot_center.y);
  printf("\n");

  // 6. 결과 검증
  printf("6️⃣ 결과 검증\n");

  // 발 중심점 일치 확인
  float foot_center_diff_x = fabs(current_foot_center.x - target_foot_center.x);
  float foot_center_diff_y = fabs(current_foot_center.y - target_foot_center.y);

  printf("  🔍 발 중심점 비교:\n");
  printf("    현재: (%.1f, %.1f)\n", current_foot_center.x,
         current_foot_center.y);
  printf("    목표: (%.1f, %.1f)\n", target_foot_center.x,
         target_foot_center.y);
  printf("    차이: (%.3f, %.3f)\n", foot_center_diff_x, foot_center_diff_y);

  if (foot_center_diff_x < 0.1f && foot_center_diff_y < 0.1f) {
    printf("    ✅ 발 중심점 완벽 일치!\n");
  } else {
    printf("    ❌ 발 중심점 불일치!\n");
  }

  // 어깨 너비 일치 확인
  float shoulder_width_diff =
      fabs(current_shoulder_width - target_shoulder_width);
  printf("  🔍 어깨 너비 비교:\n");
  printf("    현재: %.1f\n", current_shoulder_width);
  printf("    목표: %.1f\n", target_shoulder_width);
  printf("    차이: %.3f\n", shoulder_width_diff);

  if (shoulder_width_diff < 1.0f) {
    printf("    ✅ 어깨 너비 일치!\n");
  } else {
    printf("    ❌ 어깨 너비 불일치!\n");
  }

  printf("\n=== 테스트 완료 ===\n");

  segment_api_cleanup();
  return 0;
}
