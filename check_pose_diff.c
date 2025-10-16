#include "include/segment_api.h"
#include <stdio.h>

int main() {
  printf("\n🔍 mid.json 포즈 간 거리 분석\n\n");

  segment_api_init();

  // 간단한 캘리브레이션
  PoseData base;
  for (int i = 0; i < POSE_LANDMARK_COUNT; i++) {
    base.landmarks[i].position.x = 400;
    base.landmarks[i].position.y = 900 + i * 30;
    base.landmarks[i].position.z = -200;
    base.landmarks[i].inFrameLikelihood = 0.99f;
  }
  base.timestamp = 0;

  segment_calibrate_user(&base);
  segment_load_all_segments("build/mid.json");

  // 각 세그먼트 거리 확인
  for (int i = 0; i < 3; i++) {
    segment_set_current_segment(i, i + 1);
    printf("세그먼트 %d → %d 선택됨\n", i, i + 1);

    // 테스트 포즈로 진행도 확인
    float progress, similarity;
    bool is_complete;
    Point3D corrections[POSE_LANDMARK_COUNT];

    segment_analyze_simple(&base, &progress, &is_complete, &similarity,
                           corrections);
    printf("  (테스트 포즈는 무시하고 DEBUG 출력만 확인)\n\n");
  }

  return 0;
}
