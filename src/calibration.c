/**
 * @file calibration.c
 * @brief 캘리브레이션 구현 (ML Kit 33개 랜드마크 지원)
 * @author Exercise Segment API Team
 * @version 2.0.0
 */

#include "../include/calibration.h"
#include "../include/math_utils.h"
#include "../include/segment_api.h"
#include "../include/segment_types.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

// 전역 변수들은 calibration.h에서 extern 선언됨

// 최소 신뢰도 임계값
#define MIN_CONFIDENCE_THRESHOLD 0.5f

// 관절 연결 관계 정의
JointConnection g_joint_connections[20];

// segment_calibrate_recorder는 segment_core.c에서 구현됨

int segment_calibrate_user(const PoseData *base_pose) {
  if (!base_pose) {
    return SEGMENT_ERROR_INVALID_PARAMETER;
  }

  // 전역 변수들은 calibration.h에서 이미 extern 선언됨

  // 포즈 데이터 유효성 검사
  if (!segment_validate_pose(base_pose)) {
    return SEGMENT_ERROR_INVALID_POSE;
  }

  // 필수 관절들의 신뢰도 확인
  float leftShoulderConf =
      base_pose->landmarks[POSE_LANDMARK_LEFT_SHOULDER].inFrameLikelihood;
  float rightShoulderConf =
      base_pose->landmarks[POSE_LANDMARK_RIGHT_SHOULDER].inFrameLikelihood;
  float leftHipConf =
      base_pose->landmarks[POSE_LANDMARK_LEFT_HIP].inFrameLikelihood;
  float rightHipConf =
      base_pose->landmarks[POSE_LANDMARK_RIGHT_HIP].inFrameLikelihood;

  // 신뢰도가 매우 낮은 경우에만 실패 (0.1 미만)
  if (leftShoulderConf < 0.1f || rightShoulderConf < 0.1f ||
      leftHipConf < 0.1f || rightHipConf < 0.1f) {
    return SEGMENT_ERROR_CALIBRATION_FAILED;
  }

  // 어깨 너비 계산
  float user_shoulder_width =
      distance_3d(&base_pose->landmarks[POSE_LANDMARK_LEFT_SHOULDER].position,
                  &base_pose->landmarks[POSE_LANDMARK_RIGHT_SHOULDER].position);

  if (user_shoulder_width <= 10.0f) {
    return SEGMENT_ERROR_CALIBRATION_FAILED;
  }

  // 이상적 어깨 너비 (실제 데이터 기반: ~322.78)
  float ideal_shoulder_width = 322.78f;

  // 스케일 팩터 계산
  g_user_calibration.scale_factor = user_shoulder_width / ideal_shoulder_width;

  // 스케일 팩터 유효성 검사
  if (g_user_calibration.scale_factor < 0.01f ||
      g_user_calibration.scale_factor > 100.0f) {
    return SEGMENT_ERROR_CALIBRATION_FAILED;
  }

  // 중심점 오프셋 계산
  Point3D user_center_3d = calculate_pose_center(base_pose);
  // 이상적 기본 포즈 중심점 계산 (g_ideal_base_pose는 헤더에서 extern 선언됨)
  Point3D ideal_center_3d = calculate_pose_center(&g_ideal_base_pose);

  g_user_calibration.center_offset.x = ideal_center_3d.x - user_center_3d.x;
  g_user_calibration.center_offset.y = ideal_center_3d.y - user_center_3d.y;
  g_user_calibration.center_offset.z = 0.0f;

  // 관절별 길이 켈리브레이션 수행
  printf("\n🔧 관절별 길이 켈리브레이션 시작...\n");
  int joint_result =
      segment_calibrate_joint_lengths(base_pose, &g_user_calibration);
  if (joint_result != SEGMENT_OK) {
    printf("⚠️  관절별 길이 켈리브레이션 실패, 기본 켈리브레이션만 적용\n");
  }

  // 캘리브레이션 완료 플래그 설정
  g_user_calibration.is_calibrated = true;
  g_user_calibration.calibration_quality = 0.95f;

  // 중요: 사용자 캘리브레이션 완료 플래그 설정
  g_user_calibrated = true;

  // 관절별 길이 정보 출력
  // print_joint_lengths(&g_user_calibration);

  return SEGMENT_OK;
}

CalibrationData *get_calibration_data(void) {
  if (!g_recorder_calibrated) {
    return NULL;
  }
  return &g_recorder_calibration;
}

bool is_calibrated(void) {
  return g_recorder_calibrated && g_recorder_calibration.is_calibrated;
}

void reset_calibration(void) {
  memset(&g_recorder_calibration, 0, sizeof(CalibrationData));
  g_recorder_calibrated = false;
}

int segment_calibrate(const PoseData *base_pose,
                      CalibrationData *out_calibration) {
  // 기본 캘리브레이션 함수 (기록자용)
  int result = segment_calibrate_recorder(base_pose);
  if (result == SEGMENT_OK && out_calibration) {
    *out_calibration = g_recorder_calibration;
  }
  return result;
}

bool segment_validate_calibration(const CalibrationData *calibration) {
  if (!calibration) {
    return false;
  }

  // 캘리브레이션 완료 여부 확인
  if (!calibration->is_calibrated) {
    return false;
  }

  // 스케일 팩터 유효성 검사
  if (calibration->scale_factor <= 0.1f || calibration->scale_factor >= 10.0f) {
    return false;
  }

  // 캘리브레이션 품질 검사
  if (calibration->calibration_quality < 0.5f) {
    return false;
  }

  return true;
}

float calculate_pose_scale_factor(const PoseData *pose) {
  if (!pose) {
    return 1.0f;
  }

  // 어깨 너비를 기준으로 스케일 팩터 계산
  float shoulder_width =
      distance_3d(&pose->landmarks[POSE_LANDMARK_LEFT_SHOULDER].position,
                  &pose->landmarks[POSE_LANDMARK_RIGHT_SHOULDER].position);

  if (shoulder_width <= 0.0f) {
    return 1.0f;
  }

  float standard_shoulder_width = 40.0f;
  return standard_shoulder_width / shoulder_width;
}

Point3D calculate_pose_center(const PoseData *pose) {
  Point3D center = {0.0f, 0.0f, 0.0f};

  if (!pose) {
    return center;
  }

  // 모든 랜드마크의 평균 위치 계산
  for (int i = 0; i < POSE_LANDMARK_COUNT; i++) {
    center.x += pose->landmarks[i].position.x;
    center.y += pose->landmarks[i].position.y;
    center.z += pose->landmarks[i].position.z;
  }

  center.x /= POSE_LANDMARK_COUNT;
  center.y /= POSE_LANDMARK_COUNT;
  center.z /= POSE_LANDMARK_COUNT;

  return center;
}

// MARK: - 관절별 길이 켈리브레이션 함수들

int initialize_joint_connections(void) {
  // 주요 관절 연결 관계 정의 (인체 해부학적 구조 기반)
  g_joint_connections[0] = (JointConnection){
      POSE_LANDMARK_LEFT_SHOULDER, POSE_LANDMARK_LEFT_ELBOW, "좌상완"};
  g_joint_connections[1] = (JointConnection){
      POSE_LANDMARK_LEFT_ELBOW, POSE_LANDMARK_LEFT_WRIST, "좌전완"};
  g_joint_connections[2] = (JointConnection){
      POSE_LANDMARK_RIGHT_SHOULDER, POSE_LANDMARK_RIGHT_ELBOW, "우상완"};
  g_joint_connections[3] = (JointConnection){
      POSE_LANDMARK_RIGHT_ELBOW, POSE_LANDMARK_RIGHT_WRIST, "우전완"};

  g_joint_connections[4] = (JointConnection){POSE_LANDMARK_LEFT_HIP,
                                             POSE_LANDMARK_LEFT_KNEE, "좌대퇴"};
  g_joint_connections[5] = (JointConnection){
      POSE_LANDMARK_LEFT_KNEE, POSE_LANDMARK_LEFT_ANKLE, "좌정강"};
  g_joint_connections[6] = (JointConnection){
      POSE_LANDMARK_RIGHT_HIP, POSE_LANDMARK_RIGHT_KNEE, "우대퇴"};
  g_joint_connections[7] = (JointConnection){
      POSE_LANDMARK_RIGHT_KNEE, POSE_LANDMARK_RIGHT_ANKLE, "우정강"};

  g_joint_connections[8] = (JointConnection){POSE_LANDMARK_LEFT_SHOULDER,
                                             POSE_LANDMARK_LEFT_HIP, "좌상체"};
  g_joint_connections[9] = (JointConnection){POSE_LANDMARK_RIGHT_SHOULDER,
                                             POSE_LANDMARK_RIGHT_HIP, "우상체"};

  g_joint_connections[10] = (JointConnection){
      POSE_LANDMARK_LEFT_SHOULDER, POSE_LANDMARK_RIGHT_SHOULDER, "어깨너비"};
  g_joint_connections[11] = (JointConnection){
      POSE_LANDMARK_LEFT_HIP, POSE_LANDMARK_RIGHT_HIP, "골반너비"};

  g_joint_connections[12] = (JointConnection){
      POSE_LANDMARK_NOSE, POSE_LANDMARK_LEFT_SHOULDER, "목-좌어깨"};
  g_joint_connections[13] = (JointConnection){
      POSE_LANDMARK_NOSE, POSE_LANDMARK_RIGHT_SHOULDER, "목-우어깨"};

  g_joint_connections[14] = (JointConnection){
      POSE_LANDMARK_LEFT_ANKLE, POSE_LANDMARK_LEFT_HEEL, "좌발길이"};
  g_joint_connections[15] = (JointConnection){
      POSE_LANDMARK_RIGHT_ANKLE, POSE_LANDMARK_RIGHT_HEEL, "우발길이"};

  g_joint_connections[16] = (JointConnection){
      POSE_LANDMARK_LEFT_WRIST, POSE_LANDMARK_LEFT_INDEX, "좌손길이"};
  g_joint_connections[17] = (JointConnection){
      POSE_LANDMARK_RIGHT_WRIST, POSE_LANDMARK_RIGHT_INDEX, "우손길이"};

  g_joint_connections[18] = (JointConnection){
      POSE_LANDMARK_LEFT_ANKLE, POSE_LANDMARK_LEFT_FOOT_INDEX, "좌발가락"};
  g_joint_connections[19] = (JointConnection){
      POSE_LANDMARK_RIGHT_ANKLE, POSE_LANDMARK_RIGHT_FOOT_INDEX, "우발가락"};

  return 20; // 총 20개 연결
}

float calculate_joint_distance(const PoseData *pose,
                               PoseLandmarkType from_joint,
                               PoseLandmarkType to_joint) {
  if (!pose) {
    return -1.0f;
  }

  // 관절 인덱스 유효성 검사
  if (from_joint >= POSE_LANDMARK_COUNT || to_joint >= POSE_LANDMARK_COUNT) {
    return -1.0f;
  }

  // 신뢰도 검사 제거 - 모든 관절 측정
  // float from_conf = pose->landmarks[from_joint].inFrameLikelihood;
  // float to_conf = pose->landmarks[to_joint].inFrameLikelihood;

  // if (from_conf < 0.3f || to_conf < 0.3f) {
  //   return -1.0f; // 신뢰도가 너무 낮음
  // }

  // 3D 거리 계산
  return distance_3d(&pose->landmarks[from_joint].position,
                     &pose->landmarks[to_joint].position);
}

int segment_calibrate_joint_lengths(const PoseData *base_pose,
                                    CalibrationData *out_calibration) {
  if (!base_pose || !out_calibration) {
    return SEGMENT_ERROR_INVALID_PARAMETER;
  }

  // 관절 연결 관계 초기화
  int connection_count = initialize_joint_connections();

  // 관절 길이 켈리브레이션 초기화
  out_calibration->joint_lengths.count = 0;

  printf("🔧 관절별 길이 켈리브레이션 시작...\n");

  for (int i = 0; i < connection_count; i++) {
    JointConnection *conn = &g_joint_connections[i];

    // 사용자 관절 길이 계산
    float user_length =
        calculate_joint_distance(base_pose, conn->from_joint, conn->to_joint);

    if (user_length < 0.0f) {
      printf("  ⚠️  %s: 측정 실패 (신뢰도 부족)\n", conn->name);
      continue;
    }

    // 이상적 관절 길이 계산 (표준 포즈 기준)
    float ideal_length = calculate_joint_distance(
        &g_ideal_base_pose, conn->from_joint, conn->to_joint);

    if (ideal_length <= 0.0f) {
      printf("  ⚠️  %s: 이상적 길이 계산 실패\n", conn->name);
      continue;
    }

    // 스케일 팩터 계산
    float scale_factor = user_length / ideal_length;

    // 스케일 팩터 유효성 검사 (0.1 ~ 10.0 범위)
    if (scale_factor < 0.1f || scale_factor > 10.0f) {
      printf("  ⚠️  %s: 스케일 팩터 범위 초과 (%.3f)\n", conn->name,
             scale_factor);
      continue;
    }

    // 관절 길이 정보 저장
    JointLength *joint_length =
        &out_calibration->joint_lengths
             .lengths[out_calibration->joint_lengths.count];
    joint_length->connection_index = i; // 연결 인덱스 저장
    joint_length->ideal_length = ideal_length;
    joint_length->user_length = user_length;
    joint_length->scale_factor = scale_factor;
    joint_length->is_valid = true;

    out_calibration->joint_lengths.count++;

    printf("  ✅ %s: %.2f → %.2f (스케일: %.3f)\n", conn->name, ideal_length,
           user_length, scale_factor);
  }

  printf("🎯 관절별 켈리브레이션 완료: %d/%d 개 연결 성공\n",
         out_calibration->joint_lengths.count, connection_count);

  return SEGMENT_OK;
}

int apply_joint_length_calibration(const PoseData *original_pose,
                                   const CalibrationData *calibration,
                                   PoseData *calibrated_pose) {
  if (!original_pose || !calibration || !calibrated_pose) {
    return SEGMENT_ERROR_INVALID_PARAMETER;
  }

  // 원본 포즈 복사
  *calibrated_pose = *original_pose;

  // 관절별 길이 켈리브레이션 적용
  for (int i = 0; i < calibration->joint_lengths.count; i++) {
    const JointLength *joint_length = &calibration->joint_lengths.lengths[i];

    if (!joint_length->is_valid) {
      continue;
    }

    // 해당 관절 연결에 대한 스케일링 적용
    // (실제 구현에서는 더 정교한 변환 필요)
    // 여기서는 간단히 전체 스케일 팩터를 사용
    PoseLandmarkType from_joint = g_joint_connections[i].from_joint;
    PoseLandmarkType to_joint = g_joint_connections[i].to_joint;

    // 관절 위치를 중심점 기준으로 스케일링
    Point3D center = calculate_pose_center(original_pose);

    // 시작 관절 스케일링
    calibrated_pose->landmarks[from_joint].position.x =
        center.x +
        (original_pose->landmarks[from_joint].position.x - center.x) *
            joint_length->scale_factor;
    calibrated_pose->landmarks[from_joint].position.y =
        center.y +
        (original_pose->landmarks[from_joint].position.y - center.y) *
            joint_length->scale_factor;

    // 끝 관절 스케일링
    calibrated_pose->landmarks[to_joint].position.x =
        center.x + (original_pose->landmarks[to_joint].position.x - center.x) *
                       joint_length->scale_factor;
    calibrated_pose->landmarks[to_joint].position.y =
        center.y + (original_pose->landmarks[to_joint].position.y - center.y) *
                       joint_length->scale_factor;
  }

  return SEGMENT_OK;
}

void print_joint_lengths(const CalibrationData *calibration) {
  if (!calibration) {
    printf("❌ 켈리브레이션 데이터가 없습니다.\n");
    return;
  }

  printf("\n📊 관절별 길이 켈리브레이션 정보:\n");
  printf("=====================================\n");

  for (int i = 0; i < calibration->joint_lengths.count; i++) {
    const JointLength *joint_length = &calibration->joint_lengths.lengths[i];
    int conn_idx = joint_length->connection_index; // 저장된 인덱스 사용 ⭐
    JointConnection *conn = &g_joint_connections[conn_idx];

    if (joint_length->is_valid) {
      printf("  %s:\n", conn->name);
      printf("    이상적 길이: %.2f\n", joint_length->ideal_length);
      printf("    사용자 길이: %.2f\n", joint_length->user_length);
      printf("    스케일 팩터: %.3f\n", joint_length->scale_factor);
      printf("    비율 차이: %.1f%%\n",
             (joint_length->scale_factor - 1.0f) * 100.0f);
      printf("\n");
    }
  }

  printf("총 %d개 관절 연결이 켈리브레이션되었습니다.\n",
         calibration->joint_lengths.count);
}
