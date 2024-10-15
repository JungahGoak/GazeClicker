#include "GazePattern.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <iostream>
#include <assert.h>

namespace GazePattern {

HMM::HMM(int num_states, int num_observations)
    : num_states(num_states), num_observations(num_observations) {
    // Initialize transition and observation probabilities
    transition_probabilities.resize(num_states, std::vector<double>(num_states, 1.0 / num_states));
    observation_probabilities.resize(num_states, std::vector<double>(num_observations, 1.0 / num_observations));

        // 초기 전이 행렬 출력
    std::cout << "Initial Transition Matrix:" << std::endl;
    for (const auto& row : transition_probabilities) {
        for (double prob : row) {
            std::cout << prob << " ";
        }
        std::cout << std::endl;
    }

    // 초기 관측 행렬 출력
    std::cout << "Initial Observation Matrix:" << std::endl;
    for (const auto& row : observation_probabilities) {
        for (double prob : row) {
            std::cout << prob << " ";
        }
        std::cout << std::endl;
    }
}

double HMM::logsumexp(const double* array, int length) const {
    double max_val = array[0];
    for (int i = 1; i < length; i++) {
        if (array[i] > max_val) {
            max_val = array[i];
        }
    }

    double sum_exp = 0.0;
    for (int i = 0; i < length; i++) {
        sum_exp += exp(array[i] - max_val);
    }

    // Check for zero or negative sum
    if (sum_exp <= 0.0) {
        std::cerr << "Warning: logsumexp resulted in non-positive sum_exp." << std::endl;
        return max_val; // Return the max value only
    }

    return max_val + log(sum_exp);
}

void HMM::baum_welch(const std::vector<int>& obs, int n_iters) {
    int N = obs.size(); // 시퀀스 길이
    int K = num_states;

    std::cout << "baum-welch function" << std::endl;

    // log_alpha, log_beta, gamma, xi
    std::vector<std::vector<double>> log_alpha(N, std::vector<double>(K));
    std::vector<std::vector<double>> log_beta(N, std::vector<double>(K));
    std::vector<std::vector<double>> log_gamma(N, std::vector<double>(K));
    std::vector<std::vector<std::vector<double>>> log_xi(N - 1, std::vector<std::vector<double>>(K, std::vector<double>(K)));

    std::vector<std::vector<double>> gamma(N, std::vector<double>(K));
    std::vector<std::vector<std::vector<double>>> xi(N - 1, std::vector<std::vector<double>>(K, std::vector<double>(K)));

    // Baum-Welch iteration
    for (int iter = 0; iter < n_iters; iter++) {
        std::cout << "=> baum-welch iter: " << iter << std::endl;
        // E-step (forward-backward)
        // Initialize alpha
        for (int k = 0; k < K; k++) {
            log_alpha[0][k] = log(1.0 / K) + log(observation_probabilities[k][obs[0]]);
        }

        std::cout << "log_alpha[" << 0 << "] = ";
            for (int k = 0; k < K; k++) {
                std::cout << log_alpha[0][k] << " ";
            }
            std::cout << std::endl;

        // Forward pass: 어떤 시점까지의 관측된 데이터와 주어진 모델에서 특정 상태에 도달할 누적 확률
        for (int n = 1; n < N; n++) {
            for (int k = 0; k < K; k++) {
                double temp[K];
                for (int j = 0; j < K; j++) {
                    temp[j] = log_alpha[n-1][j] + log(transition_probabilities[j][k]);
                }
                log_alpha[n][k] = logsumexp(temp, K) + log(observation_probabilities[k][obs[n]]);
            }
            // 
            std::cout << "log_alpha[" << n << "] = ";
            for (int k = 0; k < K; k++) {
                std::cout << log_alpha[n][k] << " ";
            }
            std::cout << std::endl;
        }

        std::cout << "==> forward pass completed" << std::endl;

        // Backward pass:  특정 시점에서 시작하여 끝까지의 관측된 시퀀스가 주어졌을 때, 특정 상태에 있을 확률
        for (int n = N - 2; n >= 0; n--) {
            for (int k = 0; k < K; k++) {
                double temp[K];
                for (int j = 0; j < K; j++) {
                    temp[j] = log_beta[n+1][j] + log(transition_probabilities[k][j]) + log(observation_probabilities[j][obs[n+1]]);
                }
                log_beta[n][k] = logsumexp(temp, K);
            }
        }

        // 
        std::cout << "log_beta:" << std::endl;
        for (int n = 0; n < N; n++) {
            std::cout << "log_beta[" << n << "] = ";
            for (int k = 0; k < K; k++) {
                std::cout << log_beta[n][k] << " ";
            }
            std::cout << std::endl;
        }

        // gamma: 시간 t에서 상태 i에 있을 확률
        double log_evidence = logsumexp(log_alpha[N-1].data(), K);
        for (int n = 0; n < N; n++) {
            for (int k = 0; k < K; k++) {
                log_gamma[n][k] = log_alpha[n][k] + log_beta[n][k] - log_evidence;
            }
        }

        // 
        std::cout << "log_gamma:" << std::endl;
        for (int n = 0; n < N; n++) {
            std::cout << "log_gamma[" << n << "] = ";
            for (int k = 0; k < K; k++) {
                std::cout << log_gamma[n][k] << " ";
            }
            std::cout << std::endl;
        }

        
        for (int n = 0; n < N - 1; n++) {
            for (int i = 0; i < K; i++) {
                for (int j = 0; j < K; j++) {
                    log_xi[n][i][j] = log_alpha[n][i] + log(transition_probabilities[i][j]) +
                                    log(observation_probabilities[j][obs[n+1]]) +
                                    log_beta[n+1][j] - log_evidence;
                }
            }
        }

        // xi: 시간 t에서 상태 i에 있다가 시간 t+1에 상태 j로 전이할 확률
        std::cout << "xi:" << std::endl;
        for (int n = 0; n < N - 1; n++) {
            std::cout << "xi[" << n << "]:" << std::endl;
            for (int i = 0; i < K; i++) {
                for (int j = 0; j < K; j++) {
                    std::cout << log_xi[n][i][j] << " ";
                }
                std::cout << std::endl;
            }
        }

        // 로그 감마 -> 일반 감마로 변환
        for (int n = 0; n < N; n++) {
            for (int k = 0; k < K; k++) {
                log_gamma[n][k] = log_alpha[n][k] + log_beta[n][k] - log_evidence;
                gamma[n][k] = std::exp(log_gamma[n][k]);  // 일반 확률 값으로 변환
            }
        }

        // 로그 시 -> 일반 시로 변환
        for (int n = 0; n < N - 1; n++) {
            for (int i = 0; i < K; i++) {
                for (int j = 0; j < K; j++) {
                    log_xi[n][i][j] = log_alpha[n][i] + log(transition_probabilities[i][j]) +
                                      log(observation_probabilities[j][obs[n + 1]]) +
                                      log_beta[n + 1][j] - log_evidence;
                    xi[n][i][j] = std::exp(log_xi[n][i][j]);  // 일반 확률 값으로 변환
                }
            }
        }

        // M-step: Update transition and observation probabilities
        update_transition_and_observation_probabilities(gamma, xi, obs);
    }
}

void HMM::update_transition_and_observation_probabilities(const std::vector<std::vector<double>>& gamma,
                                                          const std::vector<std::vector<std::vector<double>>>& xi,
                                                          const std::vector<int>& obs) {
    int N = obs.size();
    int K = num_states;

    // 전이확률: 상태 i에서 상태j로 전이할 확률
    for (int i = 0; i < K; i++) {
        double gamma_sum = 0.0;
        for (int n = 0; n < N - 1; n++) {
            gamma_sum += gamma[n][i];
        }

        for (int j = 0; j < K; j++) {
            double xi_sum = 0.0;
            for (int n = 0; n < N - 1; n++) {
                xi_sum += xi[n][i][j];
            }
            transition_probabilities[i][j] = (xi_sum + 1e-6) / (gamma_sum + 1e-6);
        }
    }

    // 관측확률: 상태j에서 관측 기호 k가 나타날 확률
    for (int j = 0; j < K; j++) {
        double gamma_sum = 0.0;
        for (int n = 0; n < N; n++) {
            gamma_sum += gamma[n][j];
        }

        for (int v = 0; v < num_observations; v++) {
            double gamma_obs_sum = 0.0;
            for (int n = 0; n < N; n++) {
                if (obs[n] == v) {
                    gamma_obs_sum += gamma[n][j];
                }
            }
            observation_probabilities[j][v] = (gamma_obs_sum + 1e-6) / (gamma_sum + 1e-6);
        }
    }
}

void HMM::print_matrices() const {
    
    std::cout << "Transition Matrix:" << std::endl;
    for (const auto& row : transition_probabilities) {
        for (double prob : row) {
            std::cout << prob << " ";
        }
        std::cout << std::endl;
    }

    std::cout << "Observation Matrix:" << std::endl;
    for (const auto& row : observation_probabilities) {
        for (double prob : row) {
            std::cout << prob << " ";
        }
        std::cout << std::endl;
    }
}

double HMM::calculate_log_likelihood(const std::vector<int>& obs) {
    int N = obs.size(); // 관측 시퀀스 길이
    int K = num_states;

    // log_alpha 초기화
    std::vector<std::vector<double>> log_alpha(N, std::vector<double>(K));

    // 첫 번째 관측에 대한 log_alpha 값 계산
    for (int k = 0; k < K; k++) {
        log_alpha[0][k] = log(1.0 / K) + log(observation_probabilities[k][obs[0]]);
    }

    // Forward pass 수행
    for (int n = 1; n < N; n++) {
        for (int k = 0; k < K; k++) {
            double temp[K];
            for (int j = 0; j < K; j++) {
                temp[j] = log_alpha[n - 1][j] + log(transition_probabilities[j][k]);
            }
            log_alpha[n][k] = logsumexp(temp, K) + log(observation_probabilities[k][obs[n]]);
        }
        std::cout << "log_alpha[" << n << "] = ";
            for (int k = 0; k < K; k++) {
                std::cout << log_alpha[n][k] << " ";
            }
            std::cout << std::endl;
    }

    // 전체 로그-발생 확률 계산
    return logsumexp(log_alpha[N - 1].data(), K);
}

// 가장 높은 확률의 라벨과 해당 확률을 반환하는 함수
std::pair<int, double> HMM::find_most_likely_label(const std::vector<std::unique_ptr<GazePattern::HMM>>& hmm_models, const std::vector<int>& obs) {
    int best_label = -1;
    double max_log_likelihood = -std::numeric_limits<double>::infinity();
    std::vector<double> log_likelihoods;

    // HMM 목록 순회
    for (int label = 0; label < hmm_models.size(); label++) {
        if (hmm_models[label] != nullptr) {
            // 로그-발생 확률 계산
            double log_likelihood = hmm_models[label]->calculate_log_likelihood(obs);
            log_likelihoods.push_back(log_likelihood);
            std::cout << "Label " << label << " log-likelihood: " << log_likelihood << std::endl;

            // 최대 로그-발생 확률을 가진 라벨 찾기
            if (log_likelihood > max_log_likelihood) {
                max_log_likelihood = log_likelihood;
                best_label = label;
            }
        }
    }

    // 로그 우도값을 확률로 변환
    double sum_exp = 0.0;
    for (double ll : log_likelihoods) {
        sum_exp += exp(ll - max_log_likelihood);
    }

    double best_probability = exp(max_log_likelihood - max_log_likelihood) / sum_exp; // 가장 높은 확률 계산

    // 결과 반환
    return {best_label, best_probability};
}


} // namespace GazePattern