/**
 * @file segment_core.c
 * @brief 핵심 세그먼트 관리 함수들 구현
 * @author Exercise Segment API Team
 * @version 1.0.0
 */

#include "../include/calibration.h"
#include "../include/math_utils.h"
#include "../include/pose_analysis.h"
#include "../include/segment_api.h"
#include "../include/segment_types.h"
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// 전역 상태 변수들
static bool g_initialized = false;
static bool g_segment_loaded = false;

// API 내부 이상적 표준 포즈들
PoseData g_ideal_base_pose; // 이상적 기본 포즈 (extern으로 접근 가능)
static PoseData g_ideal_poses[100]; // 이상적 포즈 데이터베이스
static int g_ideal_pose_count = 0;

// A 이용자용 (기록자)
CalibrationData g_recorder_calibration; // A의 체형 데이터
bool g_recorder_calibrated = false;

// B 이용자용 (사용자)
CalibrationData g_user_calibration; // B의 체형 데이터 (extern으로 접근 가능)
bool g_user_calibrated = false;       // extern으로 접근 가능
static PoseData g_user_segment_start; // B용 변환된 시작 포즈
static PoseData g_user_segment_end;   // B용 변환된 종료 포즈

// 향상된 세그먼트 관리를 위한 전역 변수들 (v2.1.0)
static PoseData *g_user_segments = NULL; // 모든 세그먼트를 캐시하는 배열
static int g_total_segment_count = 0;      // 로드된 총 세그먼트 개수
static bool g_all_segments_loaded = false; // 전체 세그먼트 로드 여부
static int g_current_start_index = -1; // 현재 사용 중인 시작 인덱스
static int g_current_end_index = -1;   // 현재 사용 중인 종료 인덱스

// 에러 메시지 배열
static const char *error_messages[] = {"Success",
                                       "System not initialized",
                                       "Invalid pose data",
                                       "Calibration failed",
                                       "Segment not created",
                                       "Invalid parameter",
                                       "Memory allocation failed"};

// 이상적 기본 포즈 초기화 함수
static void initialize_ideal_base_pose(void) {
  // 실제 촬영된 포즈 데이터를 기반으로 한 이상적 기본 포즈 설정
  // MLKit의 33개 랜드마크 모두 설정

  // 얼굴 (11개) - 실제 데이터 기반
  g_ideal_base_pose.landmarks[POSE_LANDMARK_NOSE] =
      (PoseLandmark){{533.95f, 716.44f, -806.84f}, 0.998f};
  g_ideal_base_pose.landmarks[POSE_LANDMARK_LEFT_EYE_INNER] =
      (PoseLandmark){{551.92f, 683.25f, -781.32f}, 0.997f};
  g_ideal_base_pose.landmarks[POSE_LANDMARK_LEFT_EYE] =
      (PoseLandmark){{565.87f, 683.09f, -780.78f}, 0.997f};
  g_ideal_base_pose.landmarks[POSE_LANDMARK_LEFT_EYE_OUTER] =
      (PoseLandmark){{577.93f, 683.57f, -780.78f}, 0.996f};
  g_ideal_base_pose.landmarks[POSE_LANDMARK_RIGHT_EYE_INNER] =
      (PoseLandmark){{510.55f, 685.86f, -784.04f}, 0.997f};
  g_ideal_base_pose.landmarks[POSE_LANDMARK_RIGHT_EYE] =
      (PoseLandmark){{496.16f, 687.16f, -784.04f}, 0.996f};
  g_ideal_base_pose.landmarks[POSE_LANDMARK_RIGHT_EYE_OUTER] =
      (PoseLandmark){{482.42f, 688.38f, -783.49f}, 0.996f};
  g_ideal_base_pose.landmarks[POSE_LANDMARK_LEFT_EAR] =
      (PoseLandmark){{589.20f, 699.91f, -536.17f}, 0.996f};
  g_ideal_base_pose.landmarks[POSE_LANDMARK_RIGHT_EAR] =
      (PoseLandmark){{466.93f, 706.08f, -545.13f}, 0.996f};
  g_ideal_base_pose.landmarks[POSE_LANDMARK_MOUTH_LEFT] =
      (PoseLandmark){{560.92f, 752.43f, -700.42f}, 0.999f};
  g_ideal_base_pose.landmarks[POSE_LANDMARK_MOUTH_RIGHT] =
      (PoseLandmark){{508.23f, 752.95f, -705.31f}, 0.999f};

  // 상체 (12개) - 실제 데이터 기반
  g_ideal_base_pose.landmarks[POSE_LANDMARK_LEFT_SHOULDER] =
      (PoseLandmark){{370.82f, 919.73f, -385.50f}, 0.999f};
  g_ideal_base_pose.landmarks[POSE_LANDMARK_RIGHT_SHOULDER] =
      (PoseLandmark){{693.60f, 920.75f, -316.00f}, 0.999f};
  g_ideal_base_pose.landmarks[POSE_LANDMARK_LEFT_ELBOW] =
      (PoseLandmark){{336.08f, 1191.24f, -282.34f}, 0.990f};
  g_ideal_base_pose.landmarks[POSE_LANDMARK_RIGHT_ELBOW] =
      (PoseLandmark){{720.89f, 1193.58f, -169.68f}, 0.986f};
  g_ideal_base_pose.landmarks[POSE_LANDMARK_LEFT_WRIST] =
      (PoseLandmark){{330.49f, 1429.43f, -464.23f}, 0.971f};
  g_ideal_base_pose.landmarks[POSE_LANDMARK_RIGHT_WRIST] =
      (PoseLandmark){{722.42f, 1414.38f, -373.01f}, 0.981f};
  g_ideal_base_pose.landmarks[POSE_LANDMARK_LEFT_PINKY] =
      (PoseLandmark){{318.39f, 1502.76f, -532.92f}, 0.938f};
  g_ideal_base_pose.landmarks[POSE_LANDMARK_RIGHT_PINKY] =
      (PoseLandmark){{720.87f, 1484.23f, -432.74f}, 0.964f};
  g_ideal_base_pose.landmarks[POSE_LANDMARK_LEFT_INDEX] =
      (PoseLandmark){{342.74f, 1504.08f, -597.26f}, 0.942f};
  g_ideal_base_pose.landmarks[POSE_LANDMARK_RIGHT_INDEX] =
      (PoseLandmark){{699.98f, 1484.75f, -510.38f}, 0.968f};
  g_ideal_base_pose.landmarks[POSE_LANDMARK_LEFT_THUMB] =
      (PoseLandmark){{350.24f, 1478.91f, -491.38f}, 0.959f};
  g_ideal_base_pose.landmarks[POSE_LANDMARK_RIGHT_THUMB] =
      (PoseLandmark){{698.17f, 1457.54f, -407.76f}, 0.976f};

  // 하체 (10개) - 실제 데이터 기반
  g_ideal_base_pose.landmarks[POSE_LANDMARK_LEFT_HIP] =
      (PoseLandmark){{430.32f, 1411.64f, -31.36f}, 0.997f};
  g_ideal_base_pose.landmarks[POSE_LANDMARK_RIGHT_HIP] =
      (PoseLandmark){{615.85f, 1415.63f, 30.20f}, 0.997f};
  g_ideal_base_pose.landmarks[POSE_LANDMARK_LEFT_KNEE] =
      (PoseLandmark){{457.48f, 1767.01f, 75.61f}, 0.890f};
  g_ideal_base_pose.landmarks[POSE_LANDMARK_RIGHT_KNEE] =
      (PoseLandmark){{587.38f, 1717.72f, 165.06f}, 0.841f};
  g_ideal_base_pose.landmarks[POSE_LANDMARK_LEFT_ANKLE] =
      (PoseLandmark){{450.04f, 1991.86f, 476.72f}, 0.197f};
  g_ideal_base_pose.landmarks[POSE_LANDMARK_RIGHT_ANKLE] =
      (PoseLandmark){{573.56f, 1919.36f, 794.89f}, 0.199f};
  g_ideal_base_pose.landmarks[POSE_LANDMARK_LEFT_HEEL] =
      (PoseLandmark){{452.99f, 2026.35f, 510.11f}, 0.136f};
  g_ideal_base_pose.landmarks[POSE_LANDMARK_RIGHT_HEEL] =
      (PoseLandmark){{554.81f, 1949.91f, 855.16f}, 0.168f};
  g_ideal_base_pose.landmarks[POSE_LANDMARK_LEFT_FOOT_INDEX] = (PoseLandmark){
      {450.04f, 1991.86f, 476.72f}, 0.197f}; // 발가락 없음, 발목 재사용
  g_ideal_base_pose.landmarks[POSE_LANDMARK_RIGHT_FOOT_INDEX] = (PoseLandmark){
      {573.56f, 1919.36f, 794.89f}, 0.199f}; // 발가락 없음, 발목 재사용

  g_ideal_base_pose.timestamp = 1000;
}

// JSON 파일 처리 함수들
static int save_pose_to_json(const PoseData *pose, const char *pose_name,
                             const char *json_file_path);
static int finalize_json_workout(const char *workout_name,
                                 const char *json_file_path);
static int load_poses_from_json(const char *json_file_path, int start_index,
                                int end_index, PoseData *start_pose,
                                PoseData *end_pose);
static int parse_pose_from_json_string(const char *json_str, size_t json_len,
                                       PoseData *pose);

// JSON 파일 처리 구현
static int save_pose_to_json(const PoseData *pose, const char *pose_name,
                             const char *json_file_path) {
  if (!pose || !pose_name || !json_file_path) {
    return SEGMENT_ERROR_INVALID_PARAMETER;
  }

  // 임시 파일명 생성
  char temp_path[512];
  snprintf(temp_path, sizeof(temp_path), "%s.tmp", json_file_path);

  FILE *file = fopen(temp_path, "a");
  if (!file) {
    return SEGMENT_ERROR_MEMORY_ALLOCATION;
  }

  // JSON 형태로 포즈 데이터 저장
  fprintf(file, "  {\n");
  fprintf(file, "    \"name\": \"%s\",\n", pose_name);
  fprintf(file, "    \"timestamp\": %llu,\n", pose->timestamp);
  fprintf(file, "    \"landmarks\": [\n");

  for (int i = 0; i < POSE_LANDMARK_COUNT; i++) {
    fprintf(file, "      {\n");
    fprintf(file, "        \"index\": %d,\n", i);
    fprintf(file, "        \"position\": {\n");
    fprintf(file, "          \"x\": %.6f,\n", pose->landmarks[i].position.x);
    fprintf(file, "          \"y\": %.6f,\n", pose->landmarks[i].position.y);
    fprintf(file, "          \"z\": %.6f\n", pose->landmarks[i].position.z);
    fprintf(file, "        },\n");
    fprintf(file, "        \"confidence\": %.6f\n",
            pose->landmarks[i].inFrameLikelihood);
    fprintf(file, "      }%s\n", (i < POSE_LANDMARK_COUNT - 1) ? "," : "");
  }

  fprintf(file, "    ]\n");
  fprintf(file, "  },\n");

  fclose(file);
  return SEGMENT_OK;
}

static int finalize_json_workout(const char *workout_name,
                                 const char *json_file_path) {
  if (!workout_name || !json_file_path) {
    return SEGMENT_ERROR_INVALID_PARAMETER;
  }

  // 임시 파일에서 최종 JSON 파일로 변환
  char temp_path[512];
  snprintf(temp_path, sizeof(temp_path), "%s.tmp", json_file_path);

  FILE *temp_file = fopen(temp_path, "r");
  if (!temp_file) {
    return SEGMENT_ERROR_MEMORY_ALLOCATION;
  }

  FILE *final_file = fopen(json_file_path, "w");
  if (!final_file) {
    fclose(temp_file);
    return SEGMENT_ERROR_MEMORY_ALLOCATION;
  }

  // JSON 헤더 작성
  fprintf(final_file, "{\n");
  fprintf(final_file, "  \"workout_name\": \"%s\",\n", workout_name);
  fprintf(final_file, "  \"version\": \"2.0.0\",\n");
  fprintf(final_file, "  \"poses\": [\n");

  // 임시 파일 내용을 모두 읽어서 마지막 쉼표만 제거
  fseek(temp_file, 0, SEEK_END);
  long file_size = ftell(temp_file);
  fseek(temp_file, 0, SEEK_SET);

  char *temp_content = malloc(file_size + 1);
  if (!temp_content) {
    fclose(temp_file);
    fclose(final_file);
    return SEGMENT_ERROR_MEMORY_ALLOCATION;
  }

  fread(temp_content, 1, file_size, temp_file);
  temp_content[file_size] = '\0';

  // 마지막 쉼표 제거 (뒤에서부터 찾아서 제거)
  for (long i = file_size - 1; i >= 0; i--) {
    if (temp_content[i] == ',') {
      temp_content[i] = '\0';
      break;
    }
  }

  fprintf(final_file, "%s", temp_content);
  free(temp_content);

  // JSON 푸터 작성
  fprintf(final_file, "  ]\n");
  fprintf(final_file, "}\n");

  fclose(temp_file);
  fclose(final_file);

  // 임시 파일 삭제
  remove(temp_path);

  return SEGMENT_OK;
}

static int load_poses_from_json(const char *json_file_path, int start_index,
                                int end_index, PoseData *start_pose,
                                PoseData *end_pose) {
  if (!json_file_path || !start_pose || !end_pose) {
    printf("❌ JSON 로드 실패: NULL 포인터 (file: %p, start: %p, end: %p)\n",
           json_file_path, start_pose, end_pose);
    return SEGMENT_ERROR_INVALID_PARAMETER;
  }

  if (start_index < 0 || end_index < 0 || start_index >= end_index) {
    printf("❌ JSON 로드 실패: 잘못된 인덱스 (start: %d, end: %d)\n",
           start_index, end_index);
    return SEGMENT_ERROR_INVALID_PARAMETER;
  }

  printf("🔍 JSON 파일 로드 시작: %s (인덱스 %d → %d)\n", json_file_path,
         start_index, end_index);

  FILE *file = fopen(json_file_path, "r");
  if (!file) {
    printf("❌ JSON 파일 열기 실패: %s\n", json_file_path);
    return SEGMENT_ERROR_MEMORY_ALLOCATION;
  }

  // 개선된 JSON 파싱 - 파일 크기에 맞춰 동적 할당
  fseek(file, 0, SEEK_END);
  long file_size = ftell(file);
  fseek(file, 0, SEEK_SET);

  char *buffer = malloc(file_size + 1);
  if (!buffer) {
    fclose(file);
    printf("❌ JSON 파일용 메모리 할당 실패\n");
    return SEGMENT_ERROR_MEMORY_ALLOCATION;
  }

  size_t bytes_read = fread(buffer, 1, file_size, file);
  buffer[bytes_read] = '\0';
  fclose(file);

  if (bytes_read == 0) {
    printf("❌ JSON 파일이 비어있음: %s\n", json_file_path);
    free(buffer);
    return SEGMENT_ERROR_INVALID_PARAMETER;
  }

  printf("✅ JSON 파일 읽기 성공: %zu 바이트\n", bytes_read);

  // JSON 구조 파싱
  char *poses_start = strstr(buffer, "\"poses\"");
  if (!poses_start) {
    printf("❌ JSON에서 'poses' 배열을 찾을 수 없음\n");
    free(buffer);
    return SEGMENT_ERROR_INVALID_PARAMETER;
  }

  // poses 배열 시작점 찾기
  char *array_start = strchr(poses_start, '[');
  if (!array_start) {
    printf("❌ JSON에서 'poses' 배열 시작점 '[' 를 찾을 수 없음\n");
    free(buffer);
    return SEGMENT_ERROR_INVALID_PARAMETER;
  }

  printf("✅ 'poses' 배열 찾음, 파싱 시작\n");

  // 포즈 객체들을 순차적으로 파싱
  char *current_pos = array_start + 1;
  int current_pose_index = 0;
  bool found_start = false;
  bool found_end = false;

  while (*current_pos && (!found_start || !found_end)) {
    // 다음 포즈 객체 시작점 찾기
    char *pose_start = strchr(current_pos, '{');
    if (!pose_start)
      break;

    // 포즈 객체 끝점 찾기 (중첩된 객체 고려)
    char *pose_end = pose_start + 1;
    int brace_count = 1;
    while (*pose_end && brace_count > 0) {
      if (*pose_end == '{')
        brace_count++;
      else if (*pose_end == '}')
        brace_count--;
      pose_end++;
    }

    if (brace_count != 0)
      break; // 불완전한 JSON

    // 현재 포즈가 원하는 인덱스인지 확인
    if (current_pose_index == start_index) {
      // 시작 포즈 파싱
      memset(start_pose, 0, sizeof(PoseData));
      if (parse_pose_from_json_string(pose_start, pose_end - pose_start,
                                      start_pose) == SEGMENT_OK) {
        found_start = true;
      }
    }

    if (current_pose_index == end_index) {
      // 종료 포즈 파싱
      memset(end_pose, 0, sizeof(PoseData));
      if (parse_pose_from_json_string(pose_start, pose_end - pose_start,
                                      end_pose) == SEGMENT_OK) {
        found_end = true;
      }
    }

    current_pose_index++;
    current_pos = pose_end;

    // 다음 객체로 이동
    while (*current_pos && (*current_pos == ',' || *current_pos == ' ' ||
                            *current_pos == '\n' || *current_pos == '\t')) {
      current_pos++;
    }
  }

  if (!found_start || !found_end) {
    printf("❌ 요청한 포즈를 찾지 못함 (found_start: %s, found_end: %s, 총 "
           "포즈 수: %d)\n",
           found_start ? "예" : "아니오", found_end ? "예" : "아니오",
           current_pose_index);
    free(buffer);
    return SEGMENT_ERROR_INVALID_PARAMETER;
  }

  printf("✅ JSON 파싱 성공: 시작 포즈(%d), 종료 포즈(%d) 로드 완료\n",
         start_index, end_index);
  free(buffer);
  return SEGMENT_OK;
}

static int parse_pose_from_json_string(const char *json_str, size_t json_len,
                                       PoseData *pose) {
  if (!json_str || !pose || json_len == 0) {
    return SEGMENT_ERROR_INVALID_PARAMETER;
  }

  // JSON 문자열을 복사하여 null 종료 문자열로 만들기
  char *json_copy = malloc(json_len + 1);
  if (!json_copy) {
    return SEGMENT_ERROR_MEMORY_ALLOCATION;
  }
  strncpy(json_copy, json_str, json_len);
  json_copy[json_len] = '\0';

  // 기본값으로 초기화
  memset(pose, 0, sizeof(PoseData));
  pose->timestamp = 1000; // 기본 타임스탬프

  // timestamp 파싱
  char *timestamp_str = strstr(json_copy, "\"timestamp\"");
  if (timestamp_str) {
    char *colon = strchr(timestamp_str, ':');
    if (colon) {
      pose->timestamp = strtoull(colon + 1, NULL, 10);
    }
  }

  // landmarks 배열 파싱
  char *landmarks_start = strstr(json_copy, "\"landmarks\"");
  if (!landmarks_start) {
    free(json_copy);
    return SEGMENT_ERROR_INVALID_PARAMETER;
  }

  char *array_start = strchr(landmarks_start, '[');
  if (!array_start) {
    free(json_copy);
    return SEGMENT_ERROR_INVALID_PARAMETER;
  }

  // 각 랜드마크 파싱
  char *current_pos = array_start + 1;
  int landmark_index = 0;

  while (*current_pos && landmark_index < POSE_LANDMARK_COUNT) {
    // 다음 랜드마크 객체 찾기
    char *landmark_start = strchr(current_pos, '{');
    if (!landmark_start)
      break;

    // 랜드마크 객체 끝점 찾기
    char *landmark_end = landmark_start + 1;
    int brace_count = 1;
    while (*landmark_end && brace_count > 0) {
      if (*landmark_end == '{')
        brace_count++;
      else if (*landmark_end == '}')
        brace_count--;
      landmark_end++;
    }

    if (brace_count != 0)
      break;

    // 랜드마크 데이터 파싱
    char *x_str = strstr(landmark_start, "\"x\"");
    char *y_str = strstr(landmark_start, "\"y\"");
    char *z_str = strstr(landmark_start, "\"z\"");
    char *conf_str = strstr(landmark_start, "\"confidence\"");

    if (x_str && y_str && z_str && conf_str && x_str < landmark_end &&
        y_str < landmark_end && z_str < landmark_end &&
        conf_str < landmark_end) {

      // x 좌표 파싱
      char *x_colon = strchr(x_str, ':');
      if (x_colon) {
        pose->landmarks[landmark_index].position.x = strtof(x_colon + 1, NULL);
      }

      // y 좌표 파싱
      char *y_colon = strchr(y_str, ':');
      if (y_colon) {
        pose->landmarks[landmark_index].position.y = strtof(y_colon + 1, NULL);
      }

      // z 좌표 파싱
      char *z_colon = strchr(z_str, ':');
      if (z_colon) {
        pose->landmarks[landmark_index].position.z = strtof(z_colon + 1, NULL);
      }

      // confidence 파싱
      char *conf_colon = strchr(conf_str, ':');
      if (conf_colon) {
        pose->landmarks[landmark_index].inFrameLikelihood =
            strtof(conf_colon + 1, NULL);
      }
    }

    landmark_index++;
    current_pos = landmark_end;

    // 다음 객체로 이동
    while (*current_pos && (*current_pos == ',' || *current_pos == ' ' ||
                            *current_pos == '\n' || *current_pos == '\t')) {
      current_pos++;
    }
  }

  free(json_copy);

  // 파싱된 랜드마크 수가 충분한지 확인
  if (landmark_index < POSE_LANDMARK_COUNT / 2) {
    return SEGMENT_ERROR_INVALID_PARAMETER;
  }

  return SEGMENT_OK;
}

int segment_api_init(void) {
  if (g_initialized) {
    return SEGMENT_OK; // 이미 초기화됨
  }

  // 전역 상태 초기화
  memset(&g_ideal_base_pose, 0, sizeof(PoseData));
  memset(g_ideal_poses, 0, sizeof(g_ideal_poses));
  g_ideal_pose_count = 0;

  memset(&g_recorder_calibration, 0, sizeof(CalibrationData));
  g_recorder_calibrated = false;

  memset(&g_user_calibration, 0, sizeof(CalibrationData));
  g_user_calibrated = false;
  memset(&g_user_segment_start, 0, sizeof(PoseData));
  memset(&g_user_segment_end, 0, sizeof(PoseData));

  g_segment_loaded = false;
  g_initialized = true;

  // 이상적 기본 포즈 초기화 (표준 체형)
  initialize_ideal_base_pose();

  return SEGMENT_OK;
}

// MARK: - A 이용자 (기록자) 함수들

int segment_calibrate_recorder(const PoseData *base_pose) {
  if (!g_initialized) {
    return SEGMENT_ERROR_NOT_INITIALIZED;
  }

  if (!base_pose) {
    return SEGMENT_ERROR_INVALID_PARAMETER;
  }

  // 포즈 데이터 유효성 검사
  if (!segment_validate_pose(base_pose)) {
    return SEGMENT_ERROR_INVALID_POSE;
  }

  // 필수 관절들의 신뢰도 확인 (매우 관대하게 설정)
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
    printf("캘리브레이션 실패: 신뢰도 너무 낮음 - 어깨(L:%.2f, R:%.2f), "
           "엉덩이(L:%.2f, R:%.2f)\n",
           leftShoulderConf, rightShoulderConf, leftHipConf, rightHipConf);
    return SEGMENT_ERROR_CALIBRATION_FAILED;
  }

  printf("캘리브레이션 진행: 신뢰도 - 어깨(L:%.2f, R:%.2f), 엉덩이(L:%.2f, "
         "R:%.2f)\n",
         leftShoulderConf, rightShoulderConf, leftHipConf, rightHipConf);

  // 어깨 너비 계산
  float user_shoulder_width =
      distance_3d(&base_pose->landmarks[POSE_LANDMARK_LEFT_SHOULDER].position,
                  &base_pose->landmarks[POSE_LANDMARK_RIGHT_SHOULDER].position);

  // 어깨 너비가 너무 작거나 음수인 경우에만 실패
  if (user_shoulder_width <= 10.0f) {
    printf("캘리브레이션 실패: 어깨 너비 너무 작음 (%.2f)\n",
           user_shoulder_width);
    return SEGMENT_ERROR_CALIBRATION_FAILED;
  }

  printf("사용자 어깨 너비: %.2f (정상 범위)\n", user_shoulder_width);

  // 이상적 어깨 너비 (실제 데이터 기반: ~322.78)
  float ideal_shoulder_width = 322.78f;

  // 스케일 팩터 계산
  g_recorder_calibration.scale_factor =
      user_shoulder_width / ideal_shoulder_width;

  // 스케일 팩터 유효성 검사 (매우 관대하게 설정)
  if (g_recorder_calibration.scale_factor < 0.01f ||
      g_recorder_calibration.scale_factor > 100.0f) {
    printf("캘리브레이션 실패: 스케일 팩터 범위 초과 (%.3f)\n",
           g_recorder_calibration.scale_factor);
    return SEGMENT_ERROR_CALIBRATION_FAILED;
  }

  printf("계산된 스케일 팩터: %.3f (정상 범위)\n",
         g_recorder_calibration.scale_factor);

  // 중심점 오프셋 계산 (사용자와 이상적 포즈의 중심점 차이)
  Point3D user_center_3d = calculate_pose_center(base_pose);
  Point3D ideal_center_3d = calculate_pose_center(&g_ideal_base_pose);

  g_recorder_calibration.center_offset.x = ideal_center_3d.x - user_center_3d.x;
  g_recorder_calibration.center_offset.y = ideal_center_3d.y - user_center_3d.y;
  g_recorder_calibration.center_offset.z = 0.0f; // z는 0으로 설정

  // 캘리브레이션 완료 플래그 설정
  g_recorder_calibration.is_calibrated = true;
  g_recorder_calibration.calibration_quality = 0.95f; // 높은 품질 점수

  // 관절별 길이 켈리브레이션 수행
  printf("\n🔧 관절별 길이 켈리브레이션 시작...\n");
  int joint_result =
      segment_calibrate_joint_lengths(base_pose, &g_recorder_calibration);
  if (joint_result != SEGMENT_OK) {
    printf("⚠️  관절별 길이 켈리브레이션 실패, 기본 켈리브레이션만 적용\n");
  }

  printf("✅ 캘리브레이션 성공! 품질: %.2f\n",
         g_recorder_calibration.calibration_quality);
  printf("   - 어깨 너비: %.2f\n", user_shoulder_width);
  printf("   - 스케일 팩터: %.3f\n", g_recorder_calibration.scale_factor);
  printf("   - 중심 오프셋: (%.2f, %.2f)\n",
         g_recorder_calibration.center_offset.x,
         g_recorder_calibration.center_offset.y);

  // 관절별 길이 정보 출력
  print_joint_lengths(&g_recorder_calibration);

  g_recorder_calibrated = true;

  return SEGMENT_OK;
}

int segment_record_pose(const PoseData *current_pose, const char *pose_name,
                        const char *json_file_path) {
  if (!g_initialized) {
    return SEGMENT_ERROR_NOT_INITIALIZED;
  }

  if (!g_recorder_calibrated) {
    return SEGMENT_ERROR_CALIBRATION_FAILED;
  }

  if (!current_pose || !pose_name || !json_file_path) {
    return SEGMENT_ERROR_INVALID_PARAMETER;
  }

  // A의 포즈를 이상적 비율로 변환
  PoseData ideal_pose;
  int result = apply_calibration_to_pose(current_pose, &g_recorder_calibration,
                                         &ideal_pose);
  if (result != SEGMENT_OK) {
    return result;
  }

  // JSON 파일에 저장 (구현 필요)
  result = save_pose_to_json(&ideal_pose, pose_name, json_file_path);
  if (result != SEGMENT_OK) {
    return result;
  }

  return SEGMENT_OK;
}

int segment_finalize_workout_json(const char *workout_name,
                                  const char *json_file_path) {
  if (!g_initialized) {
    return SEGMENT_ERROR_NOT_INITIALIZED;
  }

  if (!workout_name || !json_file_path) {
    return SEGMENT_ERROR_INVALID_PARAMETER;
  }

  // JSON 파일 완성 (구현 필요)
  int result = finalize_json_workout(workout_name, json_file_path);
  if (result != SEGMENT_OK) {
    return result;
  }

  return SEGMENT_OK;
}

// MARK: - B 이용자 (사용자) 함수들

// segment_calibrate_user는 calibration.c에서 구현됨

// DEPRECATED: 이 함수는 v2.1.0에서 비효율적으로 판단되어 더 이상 권장되지
// 않습니다. 대신 segment_load_all_segments() + segment_set_current_segment()
// 조합을 사용하세요.
int segment_load_segment(const char *json_file_path, int start_index,
                         int end_index) {
  printf(
      "⚠️ DEPRECATED: segment_load_segment() 대신 segment_load_all_segments() + "
      "segment_set_current_segment() 사용을 권장합니다.\n");

  if (!g_initialized) {
    return SEGMENT_ERROR_NOT_INITIALIZED;
  }

  if (!g_user_calibrated) {
    return SEGMENT_ERROR_CALIBRATION_FAILED;
  }

  if (!json_file_path || start_index < 0 || end_index < 0) {
    return SEGMENT_ERROR_INVALID_PARAMETER;
  }

  // JSON 파일에서 포즈 로드 (구현 필요)
  PoseData ideal_start_pose, ideal_end_pose;
  int result = load_poses_from_json(json_file_path, start_index, end_index,
                                    &ideal_start_pose, &ideal_end_pose);
  if (result != SEGMENT_OK) {
    return result;
  }

  // 이상적 포즈를 B의 체형에 맞게 변환
  result = apply_calibration_to_pose(&ideal_start_pose, &g_user_calibration,
                                     &g_user_segment_start);
  if (result != SEGMENT_OK) {
    return result;
  }

  result = apply_calibration_to_pose(&ideal_end_pose, &g_user_calibration,
                                     &g_user_segment_end);
  if (result != SEGMENT_OK) {
    return result;
  }

  g_segment_loaded = true;
  return SEGMENT_OK;
}

// DEPRECATED: 이 함수는 v2.1.0에서 목표 포즈 정보를 제공하지 않아 더 이상
// 권장되지 않습니다. 대신 segment_analyze_with_target_pose() 사용을 권장합니다.
SegmentOutput segment_analyze(const PoseData *current_pose) {
  printf("⚠️ DEPRECATED: segment_analyze() 대신 "
         "segment_analyze_with_target_pose() 사용을 권장합니다.\n");

  SegmentOutput result = {0};

  if (!g_initialized || !g_segment_loaded || !current_pose) {
    return result;
  }

  // 현재 포즈와 세그먼트의 시작→종료 포즈 비교
  float progress = calculate_segment_progress(
      current_pose, &g_user_segment_start, &g_user_segment_end, NULL,
      0); // 모든 관절 사용

  float similarity =
      segment_calculate_similarity(current_pose, &g_user_segment_end);

  // 완료 판단: 유사도 기반 (앱에서 최종 판단 권장)
  bool completed = (similarity >= 0.8f);

  // 교정 벡터 계산
  calculate_correction_vectors(current_pose, &g_user_segment_end, NULL, 0,
                               result.corrections);

  result.progress = progress;
  result.completed = completed;
  result.similarity = similarity;
  result.timestamp = current_pose->timestamp;

  return result;
}

int segment_get_transformed_end_pose(PoseData *out_pose) {
  if (!g_initialized || !g_segment_loaded || !out_pose) {
    return SEGMENT_ERROR_INVALID_PARAMETER;
  }

  *out_pose = g_user_segment_end; // B의 체형에 맞게 변환된 종료 포즈
  return SEGMENT_OK;
}

// Swift 친화적인 분석 함수
int segment_analyze_simple(const PoseData *current_pose, float *out_progress,
                           bool *out_is_complete, float *out_similarity,
                           Point3D *out_corrections) {
  if (!g_initialized || !g_segment_loaded || !current_pose) {
    return SEGMENT_ERROR_INVALID_PARAMETER;
  }

  if (!out_progress || !out_is_complete || !out_similarity ||
      !out_corrections) {
    return SEGMENT_ERROR_INVALID_PARAMETER;
  }

  // 포즈 데이터 유효성 검사
  if (!segment_validate_pose(current_pose)) {
    return SEGMENT_ERROR_INVALID_POSE;
  }

  // 현재 포즈와 세그먼트의 시작→종료 포즈 비교
  float progress = calculate_segment_progress(
      current_pose, &g_user_segment_start, &g_user_segment_end, NULL, 0);

  float similarity =
      segment_calculate_similarity(current_pose, &g_user_segment_end);

  // 완료 판단: 유사도 기반 (앱에서 최종 판단 권장)
  bool completed = (similarity >= 0.8f);

  // 교정 벡터 계산
  calculate_correction_vectors(current_pose, &g_user_segment_end, NULL, 0,
                               out_corrections);

  *out_progress = progress;
  *out_is_complete = completed;
  *out_similarity = similarity;

  return SEGMENT_OK;
}

// Swift 친화적인 포즈 데이터 생성 함수
int segment_create_pose_data(const PoseLandmark *landmarks,
                             PoseData *out_pose) {
  if (!landmarks || !out_pose) {
    return SEGMENT_ERROR_INVALID_PARAMETER;
  }

  // 포즈 데이터 초기화
  memset(out_pose, 0, sizeof(PoseData));

  // 33개 랜드마크 복사
  for (int i = 0; i < POSE_LANDMARK_COUNT; i++) {
    out_pose->landmarks[i] = landmarks[i];
  }

  // 타임스탬프 설정 (현재 시간)
  out_pose->timestamp = (uint64_t)(time(NULL) * 1000);

  // 유효성 검사
  if (!segment_validate_pose(out_pose)) {
    return SEGMENT_ERROR_INVALID_POSE;
  }

  return SEGMENT_OK;
}

void segment_api_cleanup(void) {
  if (!g_initialized)
    return;

  // 세그먼트 해제
  segment_destroy();

  // 향상된 세그먼트 관리 메모리 해제 (v2.1.0)
  if (g_user_segments) {
    free(g_user_segments);
    g_user_segments = NULL;
  }
  g_total_segment_count = 0;
  g_all_segments_loaded = false;
  g_current_start_index = -1;
  g_current_end_index = -1;

  g_initialized = false;
}

// segment_calibrate와 segment_validate_calibration은 calibration.c에서 구현됨

int segment_reset(void) {
  if (!g_initialized || !g_segment_loaded) {
    return SEGMENT_ERROR_NOT_INITIALIZED;
  }

  // 세그먼트 설정은 유지하고 진행 상태만 초기화
  // 현재 구현에서는 별도의 진행 상태가 없으므로 성공 반환
  return SEGMENT_OK;
}

void segment_destroy(void) {
  g_segment_loaded = false;

  // 메모리 초기화
  memset(&g_user_segment_start, 0, sizeof(PoseData));
  memset(&g_user_segment_end, 0, sizeof(PoseData));
}

// segment_calculate_similarity와 segment_validate_pose는 pose_analysis.c에서
// 구현됨

const char *segment_get_error_message(int error_code) {
  int index = -error_code; // 에러 코드는 음수이므로 양수로 변환

  if (index >= 0 &&
      index < (int)(sizeof(error_messages) / sizeof(error_messages[0]))) {
    return error_messages[index];
  }

  return "Unknown error";
}

// MARK: - 유틸리티 함수들

bool segment_validate_pose(const PoseData *pose) {
  if (!pose) {
    return false;
  }

  // 기본적인 유효성 검사
  for (int i = 0; i < POSE_LANDMARK_COUNT; i++) {
    // 좌표값 범위 검사 (MLKit 좌표 범위에 맞게 확장)
    if (pose->landmarks[i].position.x < -10000.0f ||
        pose->landmarks[i].position.x > 10000.0f ||
        pose->landmarks[i].position.y < -10000.0f ||
        pose->landmarks[i].position.y > 10000.0f ||
        pose->landmarks[i].position.z < -10000.0f ||
        pose->landmarks[i].position.z > 10000.0f) {
      return false;
    }

    // 신뢰도 범위 검사
    if (pose->landmarks[i].inFrameLikelihood < 0.0f ||
        pose->landmarks[i].inFrameLikelihood > 1.0f) {
      return false;
    }
  }

  return true;
}

// MARK: - 캘리브레이션 관련 함수들

int apply_calibration_to_pose(const PoseData *original_pose,
                              const CalibrationData *calibration,
                              PoseData *calibrated_pose) {
  if (!original_pose || !calibrated_pose) {
    return SEGMENT_ERROR_INVALID_PARAMETER;
  }

  // 캘리브레이션 데이터가 없으면 그대로 복사
  if (!calibration || !calibration->is_calibrated) {
    *calibrated_pose = *original_pose;
    return SEGMENT_OK;
  }

  for (int i = 0; i < POSE_LANDMARK_COUNT; i++) {
    // 스케일링 적용
    calibrated_pose->landmarks[i].position.x =
        original_pose->landmarks[i].position.x * calibration->scale_factor;
    calibrated_pose->landmarks[i].position.y =
        original_pose->landmarks[i].position.y * calibration->scale_factor;
    calibrated_pose->landmarks[i].position.z =
        original_pose->landmarks[i].position.z * calibration->scale_factor;

    // 오프셋 적용
    calibrated_pose->landmarks[i].position.x += calibration->center_offset.x;
    calibrated_pose->landmarks[i].position.y += calibration->center_offset.y;
    calibrated_pose->landmarks[i].position.z += calibration->center_offset.z;

    // 신뢰도는 그대로 유지
    calibrated_pose->landmarks[i].inFrameLikelihood =
        original_pose->landmarks[i].inFrameLikelihood;
  }

  calibrated_pose->timestamp = original_pose->timestamp;
  return SEGMENT_OK;
}

// segment_calibrate 함수는 calibration.c에서 구현됨

// MARK: - Swift 호환성을 위한 함수들

int segment_calibrate_recorder_swift(const PoseData *base_pose) {
  return segment_calibrate_recorder(base_pose);
}

int segment_record_pose_swift(const PoseData *current_pose,
                              const char *pose_name,
                              const char *json_file_path) {
  return segment_record_pose(current_pose, pose_name, json_file_path);
}

// Swift에서 구조체 멤버에 접근하기 위한 헬퍼 함수들
void set_pose_landmark(PoseData *pose, int index, PoseLandmark landmark) {
  if (pose && index >= 0 && index < POSE_LANDMARK_COUNT) {
    pose->landmarks[index] = landmark;
  }
}

PoseLandmark get_pose_landmark(PoseData *pose, int index) {
  if (pose && index >= 0 && index < POSE_LANDMARK_COUNT) {
    return pose->landmarks[index];
  }

  // 기본값 반환
  PoseLandmark default_landmark = {.position = {0.0f, 0.0f, 0.0f},
                                   .inFrameLikelihood = 0.0f};
  return default_landmark;
}

// MARK: - 향상된 세그먼트 관리 API 구현 (v2.1.0)

/**
 * @brief JSON 파일에서 모든 포즈를 한 번에 로드하는 내부 함수
 */
static int load_all_poses_from_json(const char *json_file_path,
                                    PoseData **out_poses, int *out_pose_count) {
  if (!json_file_path || !out_poses || !out_pose_count) {
    printf("❌ 전체 JSON 로드 실패: NULL 포인터\n");
    return SEGMENT_ERROR_INVALID_PARAMETER;
  }

  printf("🔍 전체 JSON 파일 로드 시작: %s\n", json_file_path);

  FILE *file = fopen(json_file_path, "r");
  if (!file) {
    printf("❌ JSON 파일 열기 실패: %s\n", json_file_path);
    return SEGMENT_ERROR_MEMORY_ALLOCATION;
  }

  // 파일 크기 확인
  fseek(file, 0, SEEK_END);
  long file_size = ftell(file);
  fseek(file, 0, SEEK_SET);

  char *buffer = malloc(file_size + 1);
  if (!buffer) {
    fclose(file);
    printf("❌ JSON 파일용 메모리 할당 실패\n");
    return SEGMENT_ERROR_MEMORY_ALLOCATION;
  }

  size_t bytes_read = fread(buffer, 1, file_size, file);
  buffer[bytes_read] = '\0';
  fclose(file);

  if (bytes_read == 0) {
    printf("❌ JSON 파일이 비어있음: %s\n", json_file_path);
    free(buffer);
    return SEGMENT_ERROR_INVALID_PARAMETER;
  }

  printf("✅ JSON 파일 읽기 성공: %zu 바이트\n", bytes_read);

  // poses 배열 찾기
  char *poses_start = strstr(buffer, "\"poses\"");
  if (!poses_start) {
    printf("❌ JSON에서 'poses' 배열을 찾을 수 없음\n");
    free(buffer);
    return SEGMENT_ERROR_INVALID_PARAMETER;
  }

  char *array_start = strchr(poses_start, '[');
  if (!array_start) {
    printf("❌ JSON에서 'poses' 배열 시작점 '[' 를 찾을 수 없음\n");
    free(buffer);
    return SEGMENT_ERROR_INVALID_PARAMETER;
  }

  printf("✅ 'poses' 배열 찾음, 전체 파싱 시작\n");

  // 먼저 포즈 개수 계산
  char *temp_pos = array_start + 1;
  int pose_count = 0;
  while (*temp_pos) {
    char *pose_start = strchr(temp_pos, '{');
    if (!pose_start)
      break;

    // 포즈 객체 끝점 찾기
    char *pose_end = pose_start + 1;
    int brace_count = 1;
    while (*pose_end && brace_count > 0) {
      if (*pose_end == '{')
        brace_count++;
      else if (*pose_end == '}')
        brace_count--;
      pose_end++;
    }

    if (brace_count != 0)
      break;

    pose_count++;
    temp_pos = pose_end;

    // 다음 객체로 이동
    while (*temp_pos && (*temp_pos == ',' || *temp_pos == ' ' ||
                         *temp_pos == '\n' || *temp_pos == '\t')) {
      temp_pos++;
    }
  }

  if (pose_count == 0) {
    printf("❌ JSON에서 유효한 포즈를 찾을 수 없음\n");
    free(buffer);
    return SEGMENT_ERROR_INVALID_PARAMETER;
  }

  printf("✅ 총 %d개의 포즈 발견, 메모리 할당 중...\n", pose_count);

  // 포즈 배열 메모리 할당
  PoseData *poses = malloc(pose_count * sizeof(PoseData));
  if (!poses) {
    printf("❌ 포즈 배열 메모리 할당 실패\n");
    free(buffer);
    return SEGMENT_ERROR_MEMORY_ALLOCATION;
  }

  // 실제 포즈 파싱
  char *current_pos = array_start + 1;
  int parsed_count = 0;

  while (*current_pos && parsed_count < pose_count) {
    char *pose_start = strchr(current_pos, '{');
    if (!pose_start)
      break;

    char *pose_end = pose_start + 1;
    int brace_count = 1;
    while (*pose_end && brace_count > 0) {
      if (*pose_end == '{')
        brace_count++;
      else if (*pose_end == '}')
        brace_count--;
      pose_end++;
    }

    if (brace_count != 0)
      break;

    // 포즈 파싱
    memset(&poses[parsed_count], 0, sizeof(PoseData));
    if (parse_pose_from_json_string(pose_start, pose_end - pose_start,
                                    &poses[parsed_count]) == SEGMENT_OK) {
      parsed_count++;
    } else {
      printf("⚠️ 포즈 %d 파싱 실패, 건너뜀\n", parsed_count);
    }

    current_pos = pose_end;
    while (*current_pos && (*current_pos == ',' || *current_pos == ' ' ||
                            *current_pos == '\n' || *current_pos == '\t')) {
      current_pos++;
    }
  }

  free(buffer);

  if (parsed_count == 0) {
    printf("❌ 파싱된 포즈가 없음\n");
    free(poses);
    return SEGMENT_ERROR_INVALID_PARAMETER;
  }

  printf("✅ 전체 JSON 파싱 완료: %d개 포즈 로드 성공\n", parsed_count);

  *out_poses = poses;
  *out_pose_count = parsed_count;
  return SEGMENT_OK;
}

int segment_load_all_segments(const char *json_file_path) {
  if (!g_initialized) {
    printf("❌ API 초기화 안됨\n");
    return SEGMENT_ERROR_NOT_INITIALIZED;
  }

  if (!g_user_calibrated) {
    printf("❌ 사용자 캘리브레이션 안됨\n");
    return SEGMENT_ERROR_CALIBRATION_FAILED;
  }

  if (!json_file_path) {
    printf("❌ JSON 파일 경로가 NULL\n");
    return SEGMENT_ERROR_INVALID_PARAMETER;
  }

  printf("🚀 전체 세그먼트 로드 시작: %s\n", json_file_path);

  // 기존에 로드된 세그먼트가 있다면 해제
  if (g_user_segments) {
    free(g_user_segments);
    g_user_segments = NULL;
  }
  g_total_segment_count = 0;
  g_all_segments_loaded = false;

  // JSON에서 모든 포즈 로드
  PoseData *ideal_poses = NULL;
  int pose_count = 0;
  int result =
      load_all_poses_from_json(json_file_path, &ideal_poses, &pose_count);
  if (result != SEGMENT_OK) {
    printf("❌ JSON에서 포즈 로드 실패: 에러 코드 %d\n", result);
    return result;
  }

  // 사용자 체형에 맞게 변환된 포즈 배열 생성
  g_user_segments = malloc(pose_count * sizeof(PoseData));
  if (!g_user_segments) {
    printf("❌ 사용자 세그먼트 배열 메모리 할당 실패\n");
    free(ideal_poses);
    return SEGMENT_ERROR_MEMORY_ALLOCATION;
  }

  // 각 포즈를 사용자 체형에 맞게 변환
  printf("🔄 %d개 포즈를 사용자 체형에 맞게 변환 중...\n", pose_count);
  for (int i = 0; i < pose_count; i++) {
    result = apply_calibration_to_pose(&ideal_poses[i], &g_user_calibration,
                                       &g_user_segments[i]);
    if (result != SEGMENT_OK) {
      printf("❌ 포즈 %d 변환 실패: 에러 코드 %d\n", i, result);
      free(ideal_poses);
      free(g_user_segments);
      g_user_segments = NULL;
      return result;
    }
  }

  free(ideal_poses);

  g_total_segment_count = pose_count;
  g_all_segments_loaded = true;
  g_current_start_index = -1;
  g_current_end_index = -1;

  printf("✅ 전체 세그먼트 로드 완료: %d개 포즈가 사용자 체형에 맞게 변환됨\n",
         pose_count);
  return SEGMENT_OK;
}

int segment_set_current_segment(int start_index, int end_index) {
  if (!g_initialized) {
    printf("❌ API 초기화 안됨\n");
    return SEGMENT_ERROR_NOT_INITIALIZED;
  }

  if (!g_all_segments_loaded) {
    printf("❌ 전체 세그먼트가 로드되지 않음. segment_load_all_segments() 먼저 "
           "호출하세요\n");
    return SEGMENT_ERROR_SEGMENT_NOT_CREATED;
  }

  if (start_index < 0 || end_index < 0 ||
      start_index >= g_total_segment_count ||
      end_index >= g_total_segment_count ||
      start_index > end_index) { // 같은 인덱스 허용
    printf("❌ 잘못된 세그먼트 인덱스: start=%d, end=%d (총 %d개 포즈)\n",
           start_index, end_index, g_total_segment_count);
    return SEGMENT_ERROR_INVALID_PARAMETER;
  }

  // 현재 세그먼트 설정
  g_user_segment_start = g_user_segments[start_index];
  g_user_segment_end = g_user_segments[end_index];
  g_current_start_index = start_index;
  g_current_end_index = end_index;
  g_segment_loaded = true;

  printf("✅ 세그먼트 선택 완료: %d → %d\n", start_index, end_index);
  return SEGMENT_OK;
}

int segment_analyze_with_target_pose(const PoseData *current_pose,
                                     float *out_progress, float *out_similarity,
                                     bool *out_is_complete,
                                     Point3D *out_corrections,
                                     PoseData *out_target_pose) {
  // 기본 분석 수행
  int result =
      segment_analyze_simple(current_pose, out_progress, out_is_complete,
                             out_similarity, out_corrections);
  if (result != SEGMENT_OK) {
    return result;
  }

  // 목표 포즈 반환
  if (out_target_pose) {
    result = segment_get_transformed_end_pose(out_target_pose);
    if (result != SEGMENT_OK) {
      printf("❌ 목표 포즈 가져오기 실패: 에러 코드 %d\n", result);
      return result;
    }
  }

  return SEGMENT_OK;
}

int segment_analyze_smart(const PoseData *current_pose, ScaleMode scale_mode,
                          float screen_width, float screen_height,
                          float *out_progress, float *out_similarity,
                          bool *out_is_complete, Point3D *out_corrections,
                          PoseData *out_smart_target_pose) {

  if (!current_pose || !out_progress || !out_similarity || !out_is_complete ||
      !out_corrections || !out_smart_target_pose) {
    return SEGMENT_ERROR_INVALID_PARAMETER;
  }

  if (!g_initialized || !g_segment_loaded) {
    return SEGMENT_ERROR_NOT_INITIALIZED;
  }

  // 0. 현재 포즈의 신뢰도 체크 (팔다리 필수 랜드마크 체크)
  int valid_landmarks = 0;
  int valid_arms = 0;
  int valid_legs = 0;

  // 팔 랜드마크 체크 (어깨, 팔꿈치, 손목)
  int arm_landmarks[] = {
      POSE_LANDMARK_LEFT_SHOULDER, POSE_LANDMARK_RIGHT_SHOULDER,
      POSE_LANDMARK_LEFT_ELBOW,    POSE_LANDMARK_RIGHT_ELBOW,
      POSE_LANDMARK_LEFT_WRIST,    POSE_LANDMARK_RIGHT_WRIST};

  // 다리 랜드마크 체크 (골반, 무릎, 발목)
  int leg_landmarks[] = {POSE_LANDMARK_LEFT_HIP,   POSE_LANDMARK_RIGHT_HIP,
                         POSE_LANDMARK_LEFT_KNEE,  POSE_LANDMARK_RIGHT_KNEE,
                         POSE_LANDMARK_LEFT_ANKLE, POSE_LANDMARK_RIGHT_ANKLE};

  // 전체 랜드마크 유효성 체크
  for (int i = 0; i < POSE_LANDMARK_COUNT; i++) {
    if (current_pose->landmarks[i].inFrameLikelihood >= 0.3f) {
      valid_landmarks++;
    }
  }

  // 팔 랜드마크 유효성 체크
  for (int i = 0; i < 6; i++) {
    if (current_pose->landmarks[arm_landmarks[i]].inFrameLikelihood >= 0.3f) {
      valid_arms++;
    }
  }

  // 다리 랜드마크 유효성 체크
  for (int i = 0; i < 6; i++) {
    if (current_pose->landmarks[leg_landmarks[i]].inFrameLikelihood >= 0.3f) {
      valid_legs++;
    }
  }

  // 최소 조건: 전체 8개 이상, 팔 3개 이상, 다리 3개 이상
  if (valid_landmarks < 8 || valid_arms < 3 || valid_legs < 3) {
    *out_progress = 0.0f;
    *out_similarity = 0.0f;
    *out_is_complete = false;
    *out_smart_target_pose = g_user_segment_end; // 원본 종료 포즈 반환
    return SEGMENT_OK; // 에러가 아닌 정상적인 조기 리턴
  }

  // 1. 원본 시작 포즈와 종료 포즈 가져오기
  PoseData raw_start_pose = g_user_segment_start;
  PoseData raw_end_pose = g_user_segment_end;

  // 3. 현재 포즈의 크기 측정 (어깨 너비 기준)
  PoseLandmark current_left_shoulder =
      current_pose->landmarks[POSE_LANDMARK_LEFT_SHOULDER];
  PoseLandmark current_right_shoulder =
      current_pose->landmarks[POSE_LANDMARK_RIGHT_SHOULDER];

  float current_shoulder_width = 0;
  if (current_left_shoulder.inFrameLikelihood >= 0.5f &&
      current_right_shoulder.inFrameLikelihood >= 0.5f) {
    float dx =
        current_left_shoulder.position.x - current_right_shoulder.position.x;
    float dy =
        current_left_shoulder.position.y - current_right_shoulder.position.y;
    current_shoulder_width = sqrtf(dx * dx + dy * dy);
  } else {
    *out_smart_target_pose = raw_end_pose;
    // 스마트 목표 포즈가 원본과 같다면 원본과 비교해서 분석
    return segment_analyze_simple(current_pose, out_progress, out_is_complete,
                                  out_similarity, out_corrections);
  }

  // 2. 스케일 모드에 따른 처리
  // 두 모드 모두 스케일 조정은 함 (사용자 체형에 맞춤)
  // 차이점: 위치 변환 여부

  // 4. 종료 포즈의 크기 측정 (어깨 너비 기준)
  PoseLandmark target_left_shoulder =
      raw_end_pose.landmarks[POSE_LANDMARK_LEFT_SHOULDER];
  PoseLandmark target_right_shoulder =
      raw_end_pose.landmarks[POSE_LANDMARK_RIGHT_SHOULDER];

  float target_shoulder_width = 0;
  if (target_left_shoulder.inFrameLikelihood >= 0.5f &&
      target_right_shoulder.inFrameLikelihood >= 0.5f) {
    float dx =
        target_left_shoulder.position.x - target_right_shoulder.position.x;
    float dy =
        target_left_shoulder.position.y - target_right_shoulder.position.y;
    target_shoulder_width = sqrtf(dx * dx + dy * dy);
  } else {
    *out_smart_target_pose = raw_end_pose;
    // 스마트 목표 포즈가 원본과 같다면 원본과 비교해서 분석
    return segment_analyze_simple(current_pose, out_progress, out_is_complete,
                                  out_similarity, out_corrections);
  }

  // 5. 크기 조정을 위한 스케일 계산
  float scale = 1.0f;

  if (scale_mode == SCALE_MODE_EXERCISE) {
    // 운동 모드: 어깨 사이 거리 기준
    float current_shoulder_width = 0.0f;
    float target_shoulder_width = 0.0f;

    // 현재 포즈의 어깨 너비 계산
    if (current_left_shoulder.inFrameLikelihood >= 0.3f &&
        current_right_shoulder.inFrameLikelihood >= 0.3f) {
      float dx =
          current_left_shoulder.position.x - current_right_shoulder.position.x;
      float dy =
          current_left_shoulder.position.y - current_right_shoulder.position.y;
      current_shoulder_width = sqrtf(dx * dx + dy * dy);
    }

    // 타겟 포즈의 어깨 너비 계산
    if (target_left_shoulder.inFrameLikelihood >= 0.3f &&
        target_right_shoulder.inFrameLikelihood >= 0.3f) {
      float dx =
          target_left_shoulder.position.x - target_right_shoulder.position.x;
      float dy =
          target_left_shoulder.position.y - target_right_shoulder.position.y;
      target_shoulder_width = sqrtf(dx * dx + dy * dy);
    }

    scale = (target_shoulder_width > 0)
                ? current_shoulder_width / target_shoulder_width
                : 1.0f;

  } else {
    // 측정 모드: 어깨-발목 거리 기준 (기존 방식)
    float current_shoulder_ankle_distance = 0.0f;
    float target_shoulder_ankle_distance = 0.0f;

    // 현재 포즈의 왼쪽 발목 랜드마크 가져오기
    PoseLandmark current_left_ankle =
        current_pose->landmarks[POSE_LANDMARK_LEFT_ANKLE];

    // 현재 포즈의 왼쪽 어깨-발목 거리 계산
    if (current_left_shoulder.inFrameLikelihood >= 0.3f &&
        current_left_ankle.inFrameLikelihood >= 0.3f) {
      float dx =
          current_left_shoulder.position.x - current_left_ankle.position.x;
      float dy =
          current_left_shoulder.position.y - current_left_ankle.position.y;
      float dz =
          current_left_shoulder.position.z - current_left_ankle.position.z;
      current_shoulder_ankle_distance = sqrtf(dx * dx + dy * dy + dz * dz);
    }

    // 타겟 포즈의 왼쪽 발목 랜드마크 가져오기
    PoseLandmark target_left_ankle =
        raw_end_pose.landmarks[POSE_LANDMARK_LEFT_ANKLE];

    // 타겟 포즈의 왼쪽 어깨-발목 거리 계산
    if (target_left_shoulder.inFrameLikelihood >= 0.3f &&
        target_left_ankle.inFrameLikelihood >= 0.3f) {
      float dx = target_left_shoulder.position.x - target_left_ankle.position.x;
      float dy = target_left_shoulder.position.y - target_left_ankle.position.y;
      float dz = target_left_shoulder.position.z - target_left_ankle.position.z;
      target_shoulder_ankle_distance = sqrtf(dx * dx + dy * dy + dz * dz);
    }

    scale =
        (target_shoulder_ankle_distance > 0)
            ? current_shoulder_ankle_distance / target_shoulder_ankle_distance
            : 1.0f;
  }

  // 6. 현재 사용자의 중심점 계산 (운동 모드: 발 중심, 측정 모드: 골반 중심)
  Point3D current_center = {0};

  if (scale_mode == SCALE_MODE_EXERCISE) {
    // 운동 모드: 발과 발 사이 중심점 계산
    PoseLandmark current_left_ankle =
        current_pose->landmarks[POSE_LANDMARK_LEFT_ANKLE];
    PoseLandmark current_right_ankle =
        current_pose->landmarks[POSE_LANDMARK_RIGHT_ANKLE];

    if (current_left_ankle.inFrameLikelihood >= 0.3f &&
        current_right_ankle.inFrameLikelihood >= 0.3f) {
      current_center.x =
          (current_left_ankle.position.x + current_right_ankle.position.x) /
          2.0f;
      current_center.y =
          (current_left_ankle.position.y + current_right_ankle.position.y) /
          2.0f;
      current_center.z =
          (current_left_ankle.position.z + current_right_ankle.position.z) /
          2.0f;
    } else if (current_left_ankle.inFrameLikelihood >= 0.3f) {
      current_center = current_left_ankle.position;
    } else if (current_right_ankle.inFrameLikelihood >= 0.3f) {
      current_center = current_right_ankle.position;
    } else {
      // 발이 감지되지 않으면 골반 중심으로 대체
      PoseLandmark current_left_hip =
          current_pose->landmarks[POSE_LANDMARK_LEFT_HIP];
      PoseLandmark current_right_hip =
          current_pose->landmarks[POSE_LANDMARK_RIGHT_HIP];

      if (current_left_hip.inFrameLikelihood >= 0.3f &&
          current_right_hip.inFrameLikelihood >= 0.3f) {
        current_center.x =
            (current_left_hip.position.x + current_right_hip.position.x) / 2.0f;
        current_center.y =
            (current_left_hip.position.y + current_right_hip.position.y) / 2.0f;
        current_center.z =
            (current_left_hip.position.z + current_right_hip.position.z) / 2.0f;
      } else if (current_left_hip.inFrameLikelihood >= 0.3f) {
        current_center = current_left_hip.position;
      } else if (current_right_hip.inFrameLikelihood >= 0.3f) {
        current_center = current_right_hip.position;
      } else {
        // 그냥 원본 목표 포즈 반환
        *out_smart_target_pose = raw_end_pose;
        return segment_analyze_simple(current_pose, out_progress,
                                      out_is_complete, out_similarity,
                                      out_corrections);
      }
    }
  } else {
    // 측정 모드: 골반 중심점 계산
    PoseLandmark current_left_hip =
        current_pose->landmarks[POSE_LANDMARK_LEFT_HIP];
    PoseLandmark current_right_hip =
        current_pose->landmarks[POSE_LANDMARK_RIGHT_HIP];

    if (current_left_hip.inFrameLikelihood >= 0.3f &&
        current_right_hip.inFrameLikelihood >= 0.3f) {
      current_center.x =
          (current_left_hip.position.x + current_right_hip.position.x) / 2.0f;
      current_center.y =
          (current_left_hip.position.y + current_right_hip.position.y) / 2.0f;
      current_center.z =
          (current_left_hip.position.z + current_right_hip.position.z) / 2.0f;
    } else if (current_left_hip.inFrameLikelihood >= 0.3f) {
      current_center = current_left_hip.position;
    } else if (current_right_hip.inFrameLikelihood >= 0.3f) {
      current_center = current_right_hip.position;
    } else {
      // 그냥 원본 목표 포즈 반환
      *out_smart_target_pose = raw_end_pose;
      return segment_analyze_simple(current_pose, out_progress, out_is_complete,
                                    out_similarity, out_corrections);
    }
  }

  // 목표 포즈의 중심점 계산 (운동 모드: 발목 중심, 측정 모드: 골반 중심)
  Point3D target_center = {0};

  if (scale_mode == SCALE_MODE_EXERCISE) {
    // 운동 모드: 타겟 포즈의 발목 중심점 계산
    PoseLandmark target_left_ankle =
        raw_end_pose.landmarks[POSE_LANDMARK_LEFT_ANKLE];
    PoseLandmark target_right_ankle =
        raw_end_pose.landmarks[POSE_LANDMARK_RIGHT_ANKLE];

    if (target_left_ankle.inFrameLikelihood >= 0.3f &&
        target_right_ankle.inFrameLikelihood >= 0.3f) {
      target_center.x =
          (target_left_ankle.position.x + target_right_ankle.position.x) / 2.0f;
      target_center.y =
          (target_left_ankle.position.y + target_right_ankle.position.y) / 2.0f;
      target_center.z =
          (target_left_ankle.position.z + target_right_ankle.position.z) / 2.0f;
    } else if (target_left_ankle.inFrameLikelihood >= 0.3f) {
      target_center = target_left_ankle.position;
    } else if (target_right_ankle.inFrameLikelihood >= 0.3f) {
      target_center = target_right_ankle.position;
    } else {
      // 발목이 감지되지 않으면 골반 중심으로 대체
      PoseLandmark target_left_hip =
          raw_end_pose.landmarks[POSE_LANDMARK_LEFT_HIP];
      PoseLandmark target_right_hip =
          raw_end_pose.landmarks[POSE_LANDMARK_RIGHT_HIP];

      if (target_left_hip.inFrameLikelihood >= 0.3f &&
          target_right_hip.inFrameLikelihood >= 0.3f) {
        target_center.x =
            (target_left_hip.position.x + target_right_hip.position.x) / 2.0f;
        target_center.y =
            (target_left_hip.position.y + target_right_hip.position.y) / 2.0f;
        target_center.z =
            (target_left_hip.position.z + target_right_hip.position.z) / 2.0f;
      } else if (target_left_hip.inFrameLikelihood >= 0.3f) {
        target_center = target_left_hip.position;
      } else if (target_right_hip.inFrameLikelihood >= 0.3f) {
        target_center = target_right_hip.position;
      } else {
        *out_smart_target_pose = raw_end_pose;
        return segment_analyze_simple(current_pose, out_progress,
                                      out_is_complete, out_similarity,
                                      out_corrections);
      }
    }
  } else {
    // 측정 모드: 타겟 포즈의 골반 중심점 계산
    PoseLandmark target_left_hip =
        raw_end_pose.landmarks[POSE_LANDMARK_LEFT_HIP];
    PoseLandmark target_right_hip =
        raw_end_pose.landmarks[POSE_LANDMARK_RIGHT_HIP];

    if (target_left_hip.inFrameLikelihood >= 0.3f &&
        target_right_hip.inFrameLikelihood >= 0.3f) {
      target_center.x =
          (target_left_hip.position.x + target_right_hip.position.x) / 2.0f;
      target_center.y =
          (target_left_hip.position.y + target_right_hip.position.y) / 2.0f;
      target_center.z =
          (target_left_hip.position.z + target_right_hip.position.z) / 2.0f;
    } else if (target_left_hip.inFrameLikelihood >= 0.3f) {
      target_center = target_left_hip.position;
    } else if (target_right_hip.inFrameLikelihood >= 0.3f) {
      target_center = target_right_hip.position;
    } else {
      *out_smart_target_pose = raw_end_pose;
      return segment_analyze_simple(current_pose, out_progress, out_is_complete,
                                    out_similarity, out_corrections);
    }
  }

  // 8. 스마트 시작 포즈와 종료 포즈 생성
  PoseData smart_start_pose = raw_start_pose;
  *out_smart_target_pose = raw_end_pose;

  // 8-1. 스마트 종료 포즈 조정

  // 1단계: 타겟 포즈의 중심을 원점으로 이동
  for (int i = 0; i < POSE_LANDMARK_COUNT; i++) {
    out_smart_target_pose->landmarks[i].position.x -= target_center.x;
    out_smart_target_pose->landmarks[i].position.y -= target_center.y;
    out_smart_target_pose->landmarks[i].position.z -= target_center.z;
  }

  for (int i = 0; i < POSE_LANDMARK_COUNT; i++) {
    out_smart_target_pose->landmarks[i].position.x *= scale;
    out_smart_target_pose->landmarks[i].position.y *= scale;
    out_smart_target_pose->landmarks[i].position.z *= scale;
  }

  // 모드에 따른 위치 변환
  if (scale_mode == SCALE_MODE_EXERCISE) {
    // 운동 모드: 사용자 발 중심 따라다님
    for (int i = 0; i < POSE_LANDMARK_COUNT; i++) {
      out_smart_target_pose->landmarks[i].position.x += current_center.x;
      out_smart_target_pose->landmarks[i].position.y += current_center.y;
      out_smart_target_pose->landmarks[i].position.z += current_center.z;
    }
  } else {
    // 측정 모드: 좌우는 화면 중앙 고정, 위아래는 사용자 따라다님
    float screen_center_x = screen_width / 2.0f;

    // 포즈의 X 중심점 계산 (좌우 중앙 고정용)
    Point3D pose_center_x = {0.0f, 0.0f, 0.0f};
    int valid_landmarks = 0;

    for (int i = 0; i < POSE_LANDMARK_COUNT; i++) {
      if (out_smart_target_pose->landmarks[i].inFrameLikelihood >= 0.3f) {
        pose_center_x.x += out_smart_target_pose->landmarks[i].position.x;
        valid_landmarks++;
      }
    }

    if (valid_landmarks > 0) {
      pose_center_x.x /= valid_landmarks;
    }

    // X축만 화면 중앙에 고정, Y축은 사용자 골반 중심점 따라다님
    float offset_x = screen_center_x - pose_center_x.x;

    for (int i = 0; i < POSE_LANDMARK_COUNT; i++) {
      out_smart_target_pose->landmarks[i].position.x +=
          offset_x; // X축만 화면 중앙 고정
      out_smart_target_pose->landmarks[i].position.y +=
          current_center.y; // Y축은 사용자 중심 따라다님
      out_smart_target_pose->landmarks[i].position.z +=
          current_center.z; // Z축도 사용자 중심 따라다님
    }
  }

  // 8-2. 스마트 시작 포즈 조정
  // 시작 포즈의 중심점 계산 (운동 모드: 발목 중심, 측정 모드: 골반 중심)
  Point3D start_center = {0};

  if (scale_mode == SCALE_MODE_EXERCISE) {
    // 운동 모드: 시작 포즈의 발목 중심점 계산
    PoseLandmark start_left_ankle =
        raw_start_pose.landmarks[POSE_LANDMARK_LEFT_ANKLE];
    PoseLandmark start_right_ankle =
        raw_start_pose.landmarks[POSE_LANDMARK_RIGHT_ANKLE];

    if (start_left_ankle.inFrameLikelihood >= 0.3f &&
        start_right_ankle.inFrameLikelihood >= 0.3f) {
      start_center.x =
          (start_left_ankle.position.x + start_right_ankle.position.x) / 2.0f;
      start_center.y =
          (start_left_ankle.position.y + start_right_ankle.position.y) / 2.0f;
      start_center.z =
          (start_left_ankle.position.z + start_right_ankle.position.z) / 2.0f;
    } else if (start_left_ankle.inFrameLikelihood >= 0.3f) {
      start_center = start_left_ankle.position;
    } else if (start_right_ankle.inFrameLikelihood >= 0.3f) {
      start_center = start_right_ankle.position;
    } else {
      // 발목이 감지되지 않으면 골반 중심으로 대체
      PoseLandmark start_left_hip =
          raw_start_pose.landmarks[POSE_LANDMARK_LEFT_HIP];
      PoseLandmark start_right_hip =
          raw_start_pose.landmarks[POSE_LANDMARK_RIGHT_HIP];

      if (start_left_hip.inFrameLikelihood >= 0.3f &&
          start_right_hip.inFrameLikelihood >= 0.3f) {
        start_center.x =
            (start_left_hip.position.x + start_right_hip.position.x) / 2.0f;
        start_center.y =
            (start_left_hip.position.y + start_right_hip.position.y) / 2.0f;
        start_center.z =
            (start_left_hip.position.z + start_right_hip.position.z) / 2.0f;
      } else if (start_left_hip.inFrameLikelihood >= 0.3f) {
        start_center = start_left_hip.position;
      } else if (start_right_hip.inFrameLikelihood >= 0.3f) {
        start_center = start_right_hip.position;
      } else {
        // 시작 포즈 중심점을 종료 포즈와 동일하게 설정
        start_center = target_center;
      }
    }
  } else {
    // 측정 모드: 시작 포즈의 골반 중심점 계산
    PoseLandmark start_left_hip =
        raw_start_pose.landmarks[POSE_LANDMARK_LEFT_HIP];
    PoseLandmark start_right_hip =
        raw_start_pose.landmarks[POSE_LANDMARK_RIGHT_HIP];

    if (start_left_hip.inFrameLikelihood >= 0.3f &&
        start_right_hip.inFrameLikelihood >= 0.3f) {
      start_center.x =
          (start_left_hip.position.x + start_right_hip.position.x) / 2.0f;
      start_center.y =
          (start_left_hip.position.y + start_right_hip.position.y) / 2.0f;
      start_center.z =
          (start_left_hip.position.z + start_right_hip.position.z) / 2.0f;
    } else if (start_left_hip.inFrameLikelihood >= 0.3f) {
      start_center = start_left_hip.position;
    } else if (start_right_hip.inFrameLikelihood >= 0.3f) {
      start_center = start_right_hip.position;
    } else {
      // 시작 포즈 중심점을 종료 포즈와 동일하게 설정
      start_center = target_center;
    }
  }

  // 1단계: 시작 포즈의 중심을 원점으로 이동
  for (int i = 0; i < POSE_LANDMARK_COUNT; i++) {
    smart_start_pose.landmarks[i].position.x -= start_center.x;
    smart_start_pose.landmarks[i].position.y -= start_center.y;
    smart_start_pose.landmarks[i].position.z -= start_center.z;
  }

  // 2단계: 현재 키에 맞춰 전체 스케일 적용
  for (int i = 0; i < POSE_LANDMARK_COUNT; i++) {
    smart_start_pose.landmarks[i].position.x *= scale;
    smart_start_pose.landmarks[i].position.y *= scale;
    smart_start_pose.landmarks[i].position.z *= scale;
  }

  // 3단계: 현재 사용자의 중심 위치로 이동
  for (int i = 0; i < POSE_LANDMARK_COUNT; i++) {
    smart_start_pose.landmarks[i].position.x += current_center.x;
    smart_start_pose.landmarks[i].position.y += current_center.y;
    smart_start_pose.landmarks[i].position.z += current_center.z;
  }

  // 3. 스마트 목표 포즈와 비교해서 분석 수행
  // 포즈 데이터 유효성 검사
  if (!segment_validate_pose(current_pose)) {
    return SEGMENT_ERROR_INVALID_POSE;
  }

  // 현재 포즈와 스마트 시작 포즈 → 스마트 종료 포즈 비교
  float progress = calculate_segment_progress(current_pose, &smart_start_pose,
                                              out_smart_target_pose, NULL, 0);

  float similarity =
      segment_calculate_similarity(current_pose, out_smart_target_pose);

  // 완료 판단: 유사도 기반 (앱에서 최종 판단 권장)
  bool completed = (similarity >= 0.8f);

  // 교정 벡터 계산 (스마트 목표 포즈 기준)
  calculate_correction_vectors(current_pose, out_smart_target_pose, NULL, 0,
                               out_corrections);

  *out_progress = progress;
  *out_is_complete = completed;
  *out_similarity = similarity;

  return SEGMENT_OK;
}

int segment_get_realtime_target_pose(const PoseData *current_pose,
                                     PoseData *out_target_pose) {
  if (!g_initialized || !g_segment_loaded) {
    return SEGMENT_ERROR_NOT_INITIALIZED;
  }

  if (!current_pose || !out_target_pose) {
    return SEGMENT_ERROR_INVALID_PARAMETER;
  }

  // 현재는 단순히 변환된 종료 포즈 반환
  // 추후 사용자 위치 기반 실시간 조정 로직 추가 예정
  *out_target_pose = g_user_segment_end;

  return SEGMENT_OK;
}

int segment_get_segment_info(int *out_segment_count) {
  if (!g_initialized) {
    return SEGMENT_ERROR_NOT_INITIALIZED;
  }

  if (!out_segment_count) {
    return SEGMENT_ERROR_INVALID_PARAMETER;
  }

  *out_segment_count = g_total_segment_count;
  return SEGMENT_OK;
}
