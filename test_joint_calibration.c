/**
 * @file test_joint_calibration.c
 * @brief 관절별 길이 켈리브레이션 테스트 프로그램
 * @author Exercise Segment API Team
 * @version 1.0.0
 */

#include "include/segment_api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// 테스트용 포즈 생성 함수 (다양한 체형 시뮬레이션)
void create_test_pose(PoseData *pose, const char *body_type) {
  if (!pose)
    return;

  // 모든 랜드마크를 기본값으로 초기화
  for (int i = 0; i < POSE_LANDMARK_COUNT; i++) {
    pose->landmarks[i] = (PoseLandmark){{0.0f, 0.0f, 0.0f}, 0.9f};
  }

  if (strcmp(body_type, "tall") == 0) {
    // 키가 큰 체형 (상반신과 하반신이 모두 길음)
    printf("📏 키가 큰 체형 테스트 포즈 생성\n");

    // 얼굴
    pose->landmarks[POSE_LANDMARK_NOSE] =
        (PoseLandmark){{0.0f, -15.0f, 0.0f}, 0.9f};

    // 상체 (길게)
    pose->landmarks[POSE_LANDMARK_LEFT_SHOULDER] =
        (PoseLandmark){{-25.0f, 0.0f, 0.0f}, 0.9f};
    pose->landmarks[POSE_LANDMARK_RIGHT_SHOULDER] =
        (PoseLandmark){{25.0f, 0.0f, 0.0f}, 0.9f};
    pose->landmarks[POSE_LANDMARK_LEFT_ELBOW] =
        (PoseLandmark){{-35.0f, 35.0f, 0.0f}, 0.9f};
    pose->landmarks[POSE_LANDMARK_RIGHT_ELBOW] =
        (PoseLandmark){{35.0f, 35.0f, 0.0f}, 0.9f};
    pose->landmarks[POSE_LANDMARK_LEFT_WRIST] =
        (PoseLandmark){{-45.0f, 70.0f, 0.0f}, 0.9f};
    pose->landmarks[POSE_LANDMARK_RIGHT_WRIST] =
        (PoseLandmark){{45.0f, 70.0f, 0.0f}, 0.9f};

    // 하체 (길게)
    pose->landmarks[POSE_LANDMARK_LEFT_HIP] =
        (PoseLandmark){{-15.0f, 70.0f, 0.0f}, 0.9f};
    pose->landmarks[POSE_LANDMARK_RIGHT_HIP] =
        (PoseLandmark){{15.0f, 70.0f, 0.0f}, 0.9f};
    pose->landmarks[POSE_LANDMARK_LEFT_KNEE] =
        (PoseLandmark){{-15.0f, 120.0f, 0.0f}, 0.9f};
    pose->landmarks[POSE_LANDMARK_RIGHT_KNEE] =
        (PoseLandmark){{15.0f, 120.0f, 0.0f}, 0.9f};
    pose->landmarks[POSE_LANDMARK_LEFT_ANKLE] =
        (PoseLandmark){{-15.0f, 180.0f, 0.0f}, 0.9f};
    pose->landmarks[POSE_LANDMARK_RIGHT_ANKLE] =
        (PoseLandmark){{15.0f, 180.0f, 0.0f}, 0.9f};

  } else if (strcmp(body_type, "short_legs") == 0) {
    // 다리가 짧은 체형 (상반신은 정상, 하반신이 짧음)
    printf("📏 다리가 짧은 체형 테스트 포즈 생성\n");

    // 얼굴
    pose->landmarks[POSE_LANDMARK_NOSE] =
        (PoseLandmark){{0.0f, -10.0f, 0.0f}, 0.9f};

    // 상체 (정상)
    pose->landmarks[POSE_LANDMARK_LEFT_SHOULDER] =
        (PoseLandmark){{-20.0f, 0.0f, 0.0f}, 0.9f};
    pose->landmarks[POSE_LANDMARK_RIGHT_SHOULDER] =
        (PoseLandmark){{20.0f, 0.0f, 0.0f}, 0.9f};
    pose->landmarks[POSE_LANDMARK_LEFT_ELBOW] =
        (PoseLandmark){{-30.0f, 25.0f, 0.0f}, 0.9f};
    pose->landmarks[POSE_LANDMARK_RIGHT_ELBOW] =
        (PoseLandmark){{30.0f, 25.0f, 0.0f}, 0.9f};
    pose->landmarks[POSE_LANDMARK_LEFT_WRIST] =
        (PoseLandmark){{-40.0f, 50.0f, 0.0f}, 0.9f};
    pose->landmarks[POSE_LANDMARK_RIGHT_WRIST] =
        (PoseLandmark){{40.0f, 50.0f, 0.0f}, 0.9f};

    // 하체 (짧게)
    pose->landmarks[POSE_LANDMARK_LEFT_HIP] =
        (PoseLandmark){{-10.0f, 50.0f, 0.0f}, 0.9f};
    pose->landmarks[POSE_LANDMARK_RIGHT_HIP] =
        (PoseLandmark){{10.0f, 50.0f, 0.0f}, 0.9f};
    pose->landmarks[POSE_LANDMARK_LEFT_KNEE] =
        (PoseLandmark){{-10.0f, 70.0f, 0.0f}, 0.9f};
    pose->landmarks[POSE_LANDMARK_RIGHT_KNEE] =
        (PoseLandmark){{10.0f, 70.0f, 0.0f}, 0.9f};
    pose->landmarks[POSE_LANDMARK_LEFT_ANKLE] =
        (PoseLandmark){{-10.0f, 90.0f, 0.0f}, 0.9f};
    pose->landmarks[POSE_LANDMARK_RIGHT_ANKLE] =
        (PoseLandmark){{10.0f, 90.0f, 0.0f}, 0.9f};

  } else if (strcmp(body_type, "long_torso") == 0) {
    // 상반신이 긴 체형 (상반신이 길고, 하반신은 정상)
    printf("📏 상반신이 긴 체형 테스트 포즈 생성\n");

    // 얼굴
    pose->landmarks[POSE_LANDMARK_NOSE] =
        (PoseLandmark){{0.0f, -15.0f, 0.0f}, 0.9f};

    // 상체 (길게)
    pose->landmarks[POSE_LANDMARK_LEFT_SHOULDER] =
        (PoseLandmark){{-25.0f, 5.0f, 0.0f}, 0.9f};
    pose->landmarks[POSE_LANDMARK_RIGHT_SHOULDER] =
        (PoseLandmark){{25.0f, 5.0f, 0.0f}, 0.9f};
    pose->landmarks[POSE_LANDMARK_LEFT_ELBOW] =
        (PoseLandmark){{-35.0f, 40.0f, 0.0f}, 0.9f};
    pose->landmarks[POSE_LANDMARK_RIGHT_ELBOW] =
        (PoseLandmark){{35.0f, 40.0f, 0.0f}, 0.9f};
    pose->landmarks[POSE_LANDMARK_LEFT_WRIST] =
        (PoseLandmark){{-45.0f, 75.0f, 0.0f}, 0.9f};
    pose->landmarks[POSE_LANDMARK_RIGHT_WRIST] =
        (PoseLandmark){{45.0f, 75.0f, 0.0f}, 0.9f};

    // 하체 (정상)
    pose->landmarks[POSE_LANDMARK_LEFT_HIP] =
        (PoseLandmark){{-10.0f, 75.0f, 0.0f}, 0.9f};
    pose->landmarks[POSE_LANDMARK_RIGHT_HIP] =
        (PoseLandmark){{10.0f, 75.0f, 0.0f}, 0.9f};
    pose->landmarks[POSE_LANDMARK_LEFT_KNEE] =
        (PoseLandmark){{-10.0f, 120.0f, 0.0f}, 0.9f};
    pose->landmarks[POSE_LANDMARK_RIGHT_KNEE] =
        (PoseLandmark){{10.0f, 120.0f, 0.0f}, 0.9f};
    pose->landmarks[POSE_LANDMARK_LEFT_ANKLE] =
        (PoseLandmark){{-10.0f, 165.0f, 0.0f}, 0.9f};
    pose->landmarks[POSE_LANDMARK_RIGHT_ANKLE] =
        (PoseLandmark){{10.0f, 165.0f, 0.0f}, 0.9f};

  } else {
    // 표준 체형
    printf("📏 표준 체형 테스트 포즈 생성\n");

    // 얼굴
    pose->landmarks[POSE_LANDMARK_NOSE] =
        (PoseLandmark){{0.0f, -10.0f, 0.0f}, 0.9f};

    // 상체
    pose->landmarks[POSE_LANDMARK_LEFT_SHOULDER] =
        (PoseLandmark){{-20.0f, 0.0f, 0.0f}, 0.9f};
    pose->landmarks[POSE_LANDMARK_RIGHT_SHOULDER] =
        (PoseLandmark){{20.0f, 0.0f, 0.0f}, 0.9f};
    pose->landmarks[POSE_LANDMARK_LEFT_ELBOW] =
        (PoseLandmark){{-30.0f, 25.0f, 0.0f}, 0.9f};
    pose->landmarks[POSE_LANDMARK_RIGHT_ELBOW] =
        (PoseLandmark){{30.0f, 25.0f, 0.0f}, 0.9f};
    pose->landmarks[POSE_LANDMARK_LEFT_WRIST] =
        (PoseLandmark){{-40.0f, 50.0f, 0.0f}, 0.9f};
    pose->landmarks[POSE_LANDMARK_RIGHT_WRIST] =
        (PoseLandmark){{40.0f, 50.0f, 0.0f}, 0.9f};

    // 하체
    pose->landmarks[POSE_LANDMARK_LEFT_HIP] =
        (PoseLandmark){{-10.0f, 50.0f, 0.0f}, 0.9f};
    pose->landmarks[POSE_LANDMARK_RIGHT_HIP] =
        (PoseLandmark){{10.0f, 50.0f, 0.0f}, 0.9f};
    pose->landmarks[POSE_LANDMARK_LEFT_KNEE] =
        (PoseLandmark){{-10.0f, 100.0f, 0.0f}, 0.9f};
    pose->landmarks[POSE_LANDMARK_RIGHT_KNEE] =
        (PoseLandmark){{10.0f, 100.0f, 0.0f}, 0.9f};
    pose->landmarks[POSE_LANDMARK_LEFT_ANKLE] =
        (PoseLandmark){{-10.0f, 150.0f, 0.0f}, 0.9f};
    pose->landmarks[POSE_LANDMARK_RIGHT_ANKLE] =
        (PoseLandmark){{10.0f, 150.0f, 0.0f}, 0.9f};
  }

  // 추가 관절들 설정
  pose->landmarks[POSE_LANDMARK_LEFT_HEEL] =
      (PoseLandmark){{-12.0f, 155.0f, 0.0f}, 0.9f};
  pose->landmarks[POSE_LANDMARK_RIGHT_HEEL] =
      (PoseLandmark){{12.0f, 155.0f, 0.0f}, 0.9f};
  pose->landmarks[POSE_LANDMARK_LEFT_FOOT_INDEX] =
      (PoseLandmark){{-8.0f, 160.0f, 0.0f}, 0.9f};
  pose->landmarks[POSE_LANDMARK_RIGHT_FOOT_INDEX] =
      (PoseLandmark){{8.0f, 160.0f, 0.0f}, 0.9f};
  pose->landmarks[POSE_LANDMARK_LEFT_INDEX] =
      (PoseLandmark){{-42.0f, 55.0f, 0.0f}, 0.9f};
  pose->landmarks[POSE_LANDMARK_RIGHT_INDEX] =
      (PoseLandmark){{42.0f, 55.0f, 0.0f}, 0.9f};

  pose->timestamp = (uint64_t)time(NULL) * 1000;
}

int main() {
  printf("🧪 관절별 길이 켈리브레이션 테스트 프로그램\n");
  printf("==========================================\n\n");

  // API 초기화
  if (segment_api_init() != SEGMENT_OK) {
    printf("❌ API 초기화 실패\n");
    return -1;
  }
  printf("✅ API 초기화 성공\n\n");

  // 다양한 체형으로 테스트
  const char *body_types[] = {"standard", "tall", "short_legs", "long_torso"};
  int num_types = sizeof(body_types) / sizeof(body_types[0]);

  for (int i = 0; i < num_types; i++) {
    printf("\n🔍 테스트 %d: %s 체형\n", i + 1, body_types[i]);
    printf("=====================================\n");

    // 테스트 포즈 생성
    PoseData test_pose;
    create_test_pose(&test_pose, body_types[i]);

    // 사용자 켈리브레이션 수행
    int result = segment_calibrate_user(&test_pose);
    if (result == SEGMENT_OK) {
      printf("✅ %s 체형 켈리브레이션 성공!\n", body_types[i]);
    } else {
      printf("❌ %s 체형 켈리브레이션 실패 (에러: %d)\n", body_types[i],
             result);
    }

    printf("\n");
  }

  // API 정리
  segment_api_cleanup();
  printf("🏁 테스트 완료!\n");

  return 0;
}
