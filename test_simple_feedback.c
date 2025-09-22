#include "include/segment_api.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

// 간단한 팔 포즈 생성
void create_simple_arm_pose(PoseData *pose, float angle) {
  if (!pose)
    return;

  *pose = (PoseData){0};
  pose->timestamp = 1000;

  // 기본 포즈 설정
  pose->landmarks[POSE_LANDMARK_NOSE].position = (Point3D){400, 200, 0};
  pose->landmarks[POSE_LANDMARK_NOSE].inFrameLikelihood = 0.9f;

  // 어깨
  pose->landmarks[POSE_LANDMARK_LEFT_SHOULDER].position =
      (Point3D){300, 300, 0};
  pose->landmarks[POSE_LANDMARK_LEFT_SHOULDER].inFrameLikelihood = 0.9f;
  pose->landmarks[POSE_LANDMARK_RIGHT_SHOULDER].position =
      (Point3D){500, 300, 0};
  pose->landmarks[POSE_LANDMARK_RIGHT_SHOULDER].inFrameLikelihood = 0.9f;

  // 팔꿈치 (각도에 따라)
  float elbow_x = 400 + 50 * cos(angle);
  float elbow_y = 300 + 50 * sin(angle);
  pose->landmarks[POSE_LANDMARK_LEFT_ELBOW].position =
      (Point3D){elbow_x, elbow_y, 0};
  pose->landmarks[POSE_LANDMARK_LEFT_ELBOW].inFrameLikelihood = 0.9f;
  pose->landmarks[POSE_LANDMARK_RIGHT_ELBOW].position =
      (Point3D){elbow_x, elbow_y, 0};
  pose->landmarks[POSE_LANDMARK_RIGHT_ELBOW].inFrameLikelihood = 0.9f;

  // 손목
  float wrist_x = elbow_x + 50 * cos(angle);
  float wrist_y = elbow_y + 50 * sin(angle);
  pose->landmarks[POSE_LANDMARK_LEFT_WRIST].position =
      (Point3D){wrist_x, wrist_y, 0};
  pose->landmarks[POSE_LANDMARK_LEFT_WRIST].inFrameLikelihood = 0.9f;
  pose->landmarks[POSE_LANDMARK_RIGHT_WRIST].position =
      (Point3D){wrist_x, wrist_y, 0};
  pose->landmarks[POSE_LANDMARK_RIGHT_WRIST].inFrameLikelihood = 0.9f;

  // 하체 고정
  pose->landmarks[POSE_LANDMARK_LEFT_HIP].position = (Point3D){350, 500, 0};
  pose->landmarks[POSE_LANDMARK_LEFT_HIP].inFrameLikelihood = 0.9f;
  pose->landmarks[POSE_LANDMARK_RIGHT_HIP].position = (Point3D){450, 500, 0};
  pose->landmarks[POSE_LANDMARK_RIGHT_HIP].inFrameLikelihood = 0.9f;
  pose->landmarks[POSE_LANDMARK_LEFT_KNEE].position = (Point3D){350, 650, 0};
  pose->landmarks[POSE_LANDMARK_LEFT_KNEE].inFrameLikelihood = 0.9f;
  pose->landmarks[POSE_LANDMARK_RIGHT_KNEE].position = (Point3D){450, 650, 0};
  pose->landmarks[POSE_LANDMARK_RIGHT_KNEE].inFrameLikelihood = 0.9f;
  pose->landmarks[POSE_LANDMARK_LEFT_ANKLE].position = (Point3D){350, 800, 0};
  pose->landmarks[POSE_LANDMARK_LEFT_ANKLE].inFrameLikelihood = 0.9f;
  pose->landmarks[POSE_LANDMARK_RIGHT_ANKLE].position = (Point3D){450, 800, 0};
  pose->landmarks[POSE_LANDMARK_RIGHT_ANKLE].inFrameLikelihood = 0.9f;
}

int main() {
  // 초기화
  if (segment_api_init() != SEGMENT_OK) {
    printf("init failed\n");
    return -1;
  }

  // 캘리브레이션
  PoseData calib_pose;
  create_simple_arm_pose(&calib_pose, M_PI / 2);
  if (segment_calibrate_user(&calib_pose) != SEGMENT_OK) {
    printf("calibration failed\n");
    return -1;
  }

  // 세그먼트 로드
  if (segment_load_all_segments("test_workout.json") != SEGMENT_OK) {
    printf("load failed\n");
    return -1;
  }
  if (segment_set_current_segment(0, 0) != SEGMENT_OK) {
    printf("set failed\n");
    return -1;
  }

  // 테스트 각도들
  float angles[] = {M_PI / 6,     M_PI / 4,     M_PI / 3,    M_PI / 2,
                    2 * M_PI / 3, 3 * M_PI / 4, 5 * M_PI / 6};
  int count = sizeof(angles) / sizeof(angles[0]);

  printf("angle,progress,similarity,complete\n");

  for (int i = 0; i < count; i++) {
    // 포즈 생성
    PoseData current_pose;
    create_simple_arm_pose(&current_pose, angles[i]);

    // 분석
    float progress, similarity;
    bool is_complete;
    Point3D corrections[POSE_LANDMARK_COUNT];
    PoseData target_pose;

    int result = segment_analyze_smart(&current_pose, &progress, &similarity,
                                       &is_complete, corrections, &target_pose);

    if (result == SEGMENT_OK) {
      printf("%.2f,%.3f,%.3f,%s\n", angles[i] * 180 / M_PI, progress,
             similarity, is_complete ? "true" : "false");
    } else {
      printf("%.2f,ERROR,%d\n", angles[i] * 180 / M_PI, result);
    }

    sleep(1);
  }

  segment_api_cleanup();
  return 0;
}
