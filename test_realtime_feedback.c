#include "include/segment_api.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

// 팔만 움직이는 기본 포즈 생성 함수
void create_arm_pose(PoseData *pose, float left_arm_angle,
                     float right_arm_angle) {
  if (!pose)
    return;

  // 포즈 데이터 초기화 (test_foot_center.c 방식)
  *pose = (PoseData){0};
  pose->timestamp = 1000;

  // 기본 서있는 자세 설정 (목표 포즈와 비슷한 범위)
  pose->landmarks[POSE_LANDMARK_NOSE].position = (Point3D){200, 150, 0};
  pose->landmarks[POSE_LANDMARK_NOSE].inFrameLikelihood = 0.9f;

  // 어깨 (원래 좌표로 복원 - 캘리브레이션 성공을 위해)
  pose->landmarks[POSE_LANDMARK_LEFT_SHOULDER].position =
      (Point3D){150, 250, 0};
  pose->landmarks[POSE_LANDMARK_LEFT_SHOULDER].inFrameLikelihood = 0.9f;
  pose->landmarks[POSE_LANDMARK_RIGHT_SHOULDER].position =
      (Point3D){250, 250, 0};
  pose->landmarks[POSE_LANDMARK_RIGHT_SHOULDER].inFrameLikelihood = 0.9f;

  // 팔꿈치 (90도일 때 목표 포즈와 일치, 다른 각도에서는 점점 멀어짐)
  float left_elbow_x =
      325 + 50 * cos(left_arm_angle - M_PI / 2); // 90도일 때 325
  float left_elbow_y =
      575 + 50 * sin(left_arm_angle - M_PI / 2); // 90도일 때 575
  float right_elbow_x = 325 + 50 * cos(right_arm_angle - M_PI / 2);
  float right_elbow_y = 575 + 50 * sin(right_arm_angle - M_PI / 2);

  pose->landmarks[POSE_LANDMARK_LEFT_ELBOW].position =
      (Point3D){left_elbow_x, left_elbow_y, 0};
  pose->landmarks[POSE_LANDMARK_LEFT_ELBOW].inFrameLikelihood = 0.9f;
  pose->landmarks[POSE_LANDMARK_RIGHT_ELBOW].position =
      (Point3D){right_elbow_x, right_elbow_y, 0};
  pose->landmarks[POSE_LANDMARK_RIGHT_ELBOW].inFrameLikelihood = 0.9f;

  // 손목 (90도일 때 목표 포즈와 일치, 다른 각도에서는 점점 멀어짐)
  float left_wrist_x =
      300 + 50 * cos(left_arm_angle - M_PI / 2); // 90도일 때 300
  float left_wrist_y =
      625 + 50 * sin(left_arm_angle - M_PI / 2); // 90도일 때 625
  float right_wrist_x = 300 + 50 * cos(right_arm_angle - M_PI / 2);
  float right_wrist_y = 625 + 50 * sin(right_arm_angle - M_PI / 2);

  pose->landmarks[POSE_LANDMARK_LEFT_WRIST].position =
      (Point3D){left_wrist_x, left_wrist_y, 0};
  pose->landmarks[POSE_LANDMARK_LEFT_WRIST].inFrameLikelihood = 0.9f;
  pose->landmarks[POSE_LANDMARK_RIGHT_WRIST].position =
      (Point3D){right_wrist_x, right_wrist_y, 0};
  pose->landmarks[POSE_LANDMARK_RIGHT_WRIST].inFrameLikelihood = 0.9f;

  // 하체는 고정 (엉덩이, 무릎, 발목) - 원래 좌표로 복원
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

// 특정 랜드마크의 교정 벡터 출력
void print_correction_for_landmark(const Point3D *corrections,
                                   int landmark_index,
                                   const char *landmark_name) {
  if (landmark_index < POSE_LANDMARK_COUNT) {
    printf("  %s: (%.2f, %.2f, %.2f)\n", landmark_name,
           corrections[landmark_index].x, corrections[landmark_index].y,
           corrections[landmark_index].z);
  }
}

int main() {
  // 초기화
  if (segment_api_init() != SEGMENT_OK) {
    printf("init failed\n");
    return -1;
  }

  // 캘리브레이션
  PoseData calibration_pose;
  create_arm_pose(&calibration_pose, M_PI / 2, M_PI / 2);
  if (segment_calibrate_user(&calibration_pose) != SEGMENT_OK) {
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

  float test_angles[] = {
      M_PI / 6, // 30도 - 잘못된 각도
      M_PI / 4, // 45도 - 조금 더 가까운 각도
      M_PI / 3, // 60도 - 더 가까운 각도
      M_PI / 2, // 90도 - 목표 각도
      M_PI / 3, // 60도 - 다시 멀어짐
      M_PI / 6, // 30도 - 다시 멀어짐
  };

  int angle_count = sizeof(test_angles) / sizeof(test_angles[0]);

  printf("=== INPUT - OUTPUT ===\n");

  for (int i = 0; i < angle_count; i++) {
    float current_angle = test_angles[i];

    // 현재 포즈 생성
    PoseData current_pose;
    create_arm_pose(&current_pose, current_angle, current_angle);

    // Smart 분석 수행
    float progress, similarity;
    bool is_complete;
    Point3D corrections[POSE_LANDMARK_COUNT];
    PoseData target_pose;

    int result = segment_analyze_smart(&current_pose, &progress, &similarity,
                                       &is_complete, corrections, &target_pose);

    if (result == SEGMENT_OK) {
      // 입력값 (팔꿈치, 손목 좌표)
      float elbow_x = 325 + 50 * cos(current_angle - M_PI / 2);
      float elbow_y = 575 + 50 * sin(current_angle - M_PI / 2);
      float wrist_x = 300 + 50 * cos(current_angle - M_PI / 2);
      float wrist_y = 625 + 50 * sin(current_angle - M_PI / 2);

      // 목표 포즈의 다른 관절들도 출력
      printf("INPUT: angle=%.1f elbow=(%.1f,%.1f) wrist=(%.1f,%.1f) | TARGET: "
             "elbow=(%.1f,%.1f) wrist=(%.1f,%.1f) shoulder=(%.1f,%.1f) "
             "hip=(%.1f,%.1f) | CORRECTIONS: "
             "elbow=(%.1f,%.1f,%.1f) wrist=(%.1f,%.1f,%.1f) | OUTPUT: "
             "progress=%.3f "
             "similarity=%.3f complete=%s\n",
             current_angle * 180 / M_PI, elbow_x, elbow_y, wrist_x, wrist_y,
             target_pose.landmarks[POSE_LANDMARK_LEFT_ELBOW].position.x,
             target_pose.landmarks[POSE_LANDMARK_LEFT_ELBOW].position.y,
             target_pose.landmarks[POSE_LANDMARK_LEFT_WRIST].position.x,
             target_pose.landmarks[POSE_LANDMARK_LEFT_WRIST].position.y,
             target_pose.landmarks[POSE_LANDMARK_LEFT_SHOULDER].position.x,
             target_pose.landmarks[POSE_LANDMARK_LEFT_SHOULDER].position.y,
             target_pose.landmarks[POSE_LANDMARK_LEFT_HIP].position.x,
             target_pose.landmarks[POSE_LANDMARK_LEFT_HIP].position.y,
             corrections[POSE_LANDMARK_LEFT_ELBOW].x,
             corrections[POSE_LANDMARK_LEFT_ELBOW].y,
             corrections[POSE_LANDMARK_LEFT_ELBOW].z,
             corrections[POSE_LANDMARK_LEFT_WRIST].x,
             corrections[POSE_LANDMARK_LEFT_WRIST].y,
             corrections[POSE_LANDMARK_LEFT_WRIST].z, progress, similarity,
             is_complete ? "true" : "false");
    } else {
      printf("INPUT: angle=%.1f | OUTPUT: ERROR %d\n",
             current_angle * 180 / M_PI, result);
    }

    sleep(1);
  }

  printf("\n=== 테스트 완료 ===\n");
  segment_api_cleanup();
  return 0;
}
