/**
 * @file smart_demo.c
 * @brief 사용자 위치 기준 포즈 분석 API 데모
 * @author Exercise Segment API Team
 *
 * 현재 포즈를 입력하면 사용자 위치를 기준으로 조정된 목표 포즈를 반환합니다.
 */

#include "../include/segment_api.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief 다양한 위치의 테스트 포즈 생성
 */
PoseData create_pose_at_position(float center_x, float center_y) {
  PoseData pose = {0};

  // 지정된 중심점 기준으로 포즈 생성
  pose.landmarks[POSE_LANDMARK_NOSE] =
      (PoseLandmark){{center_x, center_y - 200.0f, 0.0f}, 0.9f};
  pose.landmarks[POSE_LANDMARK_LEFT_SHOULDER] =
      (PoseLandmark){{center_x - 50.0f, center_y - 100.0f, 0.0f}, 0.95f};
  pose.landmarks[POSE_LANDMARK_RIGHT_SHOULDER] =
      (PoseLandmark){{center_x + 50.0f, center_y - 100.0f, 0.0f}, 0.95f};
  pose.landmarks[POSE_LANDMARK_LEFT_ELBOW] =
      (PoseLandmark){{center_x - 80.0f, center_y, 0.0f}, 0.9f};
  pose.landmarks[POSE_LANDMARK_RIGHT_ELBOW] =
      (PoseLandmark){{center_x + 80.0f, center_y, 0.0f}, 0.9f};
  pose.landmarks[POSE_LANDMARK_LEFT_WRIST] =
      (PoseLandmark){{center_x - 100.0f, center_y + 100.0f, 0.0f}, 0.85f};
  pose.landmarks[POSE_LANDMARK_RIGHT_WRIST] =
      (PoseLandmark){{center_x + 100.0f, center_y + 100.0f, 0.0f}, 0.85f};
  pose.landmarks[POSE_LANDMARK_LEFT_HIP] =
      (PoseLandmark){{center_x - 30.0f, center_y + 200.0f, 0.0f}, 0.95f};
  pose.landmarks[POSE_LANDMARK_RIGHT_HIP] =
      (PoseLandmark){{center_x + 30.0f, center_y + 200.0f, 0.0f}, 0.95f};
  pose.landmarks[POSE_LANDMARK_LEFT_KNEE] =
      (PoseLandmark){{center_x - 40.0f, center_y + 400.0f, 0.0f}, 0.9f};
  pose.landmarks[POSE_LANDMARK_RIGHT_KNEE] =
      (PoseLandmark){{center_x + 40.0f, center_y + 400.0f, 0.0f}, 0.9f};
  pose.landmarks[POSE_LANDMARK_LEFT_ANKLE] =
      (PoseLandmark){{center_x - 50.0f, center_y + 600.0f, 0.0f}, 0.85f};
  pose.landmarks[POSE_LANDMARK_RIGHT_ANKLE] =
      (PoseLandmark){{center_x + 50.0f, center_y + 600.0f, 0.0f}, 0.85f};

  pose.timestamp = 1000;
  return pose;
}

/**
 * @brief 포즈 정보를 간단히 출력
 */
void print_pose_summary(const PoseData *pose, const char *title) {
  printf("\n📍 %s:\n", title);
  printf("  👃 코: (%.1f, %.1f)\n",
         pose->landmarks[POSE_LANDMARK_NOSE].position.x,
         pose->landmarks[POSE_LANDMARK_NOSE].position.y);
  printf("  👐 어깨: L(%.1f, %.1f) R(%.1f, %.1f)\n",
         pose->landmarks[POSE_LANDMARK_LEFT_SHOULDER].position.x,
         pose->landmarks[POSE_LANDMARK_LEFT_SHOULDER].position.y,
         pose->landmarks[POSE_LANDMARK_RIGHT_SHOULDER].position.x,
         pose->landmarks[POSE_LANDMARK_RIGHT_SHOULDER].position.y);
  printf("  🦶 발목: L(%.1f, %.1f) R(%.1f, %.1f)\n",
         pose->landmarks[POSE_LANDMARK_LEFT_ANKLE].position.x,
         pose->landmarks[POSE_LANDMARK_LEFT_ANKLE].position.y,
         pose->landmarks[POSE_LANDMARK_RIGHT_ANKLE].position.x,
         pose->landmarks[POSE_LANDMARK_RIGHT_ANKLE].position.y);
}

int main(void) {
  printf("🧠 똑똑한 API 데모 - 사용자 위치 기준 목표 포즈!\n");
  printf("===============================================\n\n");

  // 1. API 초기화
  printf("1️⃣ API 초기화...\n");
  int result = segment_api_init();
  if (result != SEGMENT_OK) {
    printf("❌ API 초기화 실패: %s\n", segment_get_error_message(result));
    return 1;
  }
  printf("✅ API 초기화 완료\n\n");

  // 2. 사용자 캘리브레이션
  printf("2️⃣ 사용자 캘리브레이션...\n");
  PoseData base_pose = create_pose_at_position(400.0f, 400.0f);
  result = segment_calibrate_user(&base_pose);
  if (result != SEGMENT_OK) {
    printf("❌ 사용자 캘리브레이션 실패: %s\n",
           segment_get_error_message(result));
    segment_api_cleanup();
    return 1;
  }
  printf("✅ 사용자 캘리브레이션 완료\n\n");

  // 3. 전체 세그먼트 로드
  printf("3️⃣ 전체 세그먼트 로드...\n");
  const char *json_file = "test_workout.json";
  result = segment_load_all_segments(json_file);
  if (result != SEGMENT_OK) {
    printf("❌ 전체 세그먼트 로드 실패: %s\n",
           segment_get_error_message(result));
    segment_api_cleanup();
    return 1;
  }
  printf("✅ 전체 세그먼트 로드 완료\n\n");

  // 4. 세그먼트 선택
  printf("4️⃣ 세그먼트 선택...\n");
  result = segment_set_current_segment(0, 0);
  if (result != SEGMENT_OK) {
    printf("❌ 세그먼트 선택 실패: %s\n", segment_get_error_message(result));
    segment_api_cleanup();
    return 1;
  }
  printf("✅ 세그먼트 선택 완료\n\n");

  printf("🧠 **똑똑한 분석 테스트 시작!**\n");
  printf("===============================================\n");

  // 5. 다양한 위치에서 테스트
  float test_positions[][2] = {
      {200.0f, 300.0f}, // 왼쪽 위
      {600.0f, 300.0f}, // 오른쪽 위
      {400.0f, 100.0f}, // 중앙 위
      {400.0f, 500.0f}, // 중앙 아래
      {100.0f, 700.0f}  // 왼쪽 아래
  };

  const char *position_names[] = {"👈 왼쪽 위", "👉 오른쪽 위", "⬆️ 중앙 위",
                                  "⬇️ 중앙 아래", "👇 왼쪽 아래"};

  for (int i = 0; i < 5; i++) {
    printf("\n📍 **테스트 %d: %s 위치**\n", i + 1, position_names[i]);
    printf("─────────────────────────────────────\n");

    // 해당 위치에서 사용자 포즈 생성
    PoseData current_pose =
        create_pose_at_position(test_positions[i][0], test_positions[i][1]);
    print_pose_summary(&current_pose, "현재 사용자 포즈");

    // 🧠 똑똑한 분석 실행!
    float progress, similarity;
    bool is_complete;
    Point3D corrections[POSE_LANDMARK_COUNT];
    PoseData smart_target_pose = {0};

    result =
        segment_analyze_smart(&current_pose, // 현재 포즈만 넣으면
                              &progress,     // 분석 결과들과
                              &similarity, &is_complete, corrections,
                              &smart_target_pose // 사용자 위치 기준 목표 포즈!
        );

    if (result == SEGMENT_OK) {
      print_pose_summary(&smart_target_pose,
                         "🎯 똑똑한 목표 포즈 (사용자 위치 기준)");

      // 발목 위치가 얼마나 가까운지 확인
      float current_foot_x =
          (current_pose.landmarks[POSE_LANDMARK_LEFT_ANKLE].position.x +
           current_pose.landmarks[POSE_LANDMARK_RIGHT_ANKLE].position.x) /
          2.0f;
      float target_foot_x =
          (smart_target_pose.landmarks[POSE_LANDMARK_LEFT_ANKLE].position.x +
           smart_target_pose.landmarks[POSE_LANDMARK_RIGHT_ANKLE].position.x) /
          2.0f;

      printf("  📏 발 중심점 일치도: 현재(%.1f) vs 목표(%.1f) = 차이 %.1f\n",
             current_foot_x, target_foot_x,
             fabs(current_foot_x - target_foot_x));

      if (fabs(current_foot_x - target_foot_x) < 10.0f) {
        printf("  ✅ 완벽하게 사용자 위치에 맞춰짐!\n");
      }
    } else {
      printf("❌ 똑똑한 분석 실패: %s\n", segment_get_error_message(result));
    }
  }

  printf("\n===============================================\n");
  printf("🎉 **똑똑한 API 데모 완료!**\n\n");

  printf("💡 **핵심 장점:**\n");
  printf("  - 사용자가 어디에 있든 목표 포즈가 자동으로 따라옴! "
         "🏃‍♂️\n");
  printf("  - 화면 크기나 해상도 상관없음! 📱💻\n");
  printf("  - 자연스러운 코칭 경험! 🎯\n\n");

  printf("🚀 **사용법:**\n");
  printf("```c\n");
  printf("// 현재 포즈만 넣으면\n");
  printf("segment_analyze_smart(current_pose, &progress, &similarity,\n");
  printf("                     &complete, corrections, &target_pose);\n");
  printf("// → 사용자 위치 기준으로 딱 맞춰진 목표 포즈 완성!\n");
  printf("```\n\n");

  // 6. 정리
  printf("6️⃣ API 정리...\n");
  segment_api_cleanup();
  printf("✅ API 정리 완료\n");

  return 0;
}
