#include "numerical_flux.h"

NumericalFlux::NumericalFlux(double kappa) : kappa(kappa)
{
}

void NumericalFlux::Q(double result[4], double state_vector[4], double nx, double ny)
{
  result[0] = state_vector[0];
  double temp_result_1 = nx * state_vector[1] + ny * state_vector[2];
  double temp_result_2 = -ny * state_vector[1] + nx * state_vector[2];
  result[1] = temp_result_1;
  result[2] = temp_result_2;
  result[3] = state_vector[3];
}

void NumericalFlux::Q_inv(double result[4], double state_vector[4], double nx, double ny)
{
  result[0] = state_vector[0];
  double temp_result_1 = nx * state_vector[1] - ny * state_vector[2];
  double temp_result_2 = ny * state_vector[1] + nx * state_vector[2];
  result[1] = temp_result_1;
  result[2] = temp_result_2;
  result[3] = state_vector[3];
}

void NumericalFlux::f_1(double result[4], double state[4])
{
  result[0] = state[1];
  result[1] = state[1] * state[1] / state[0] + QuantityCalculator::calc_pressure(state[0], state[1], state[2], state[3], kappa);
  result[2] = state[2] * state[1] / state[0];
  result[3] = (state[1] / state[0]) * (state[3] + QuantityCalculator::calc_pressure(state[0], state[1], state[2], state[3], kappa));
}


VijayasundaramNumericalFlux::VijayasundaramNumericalFlux() : NumericalFlux(0)
{
}

void VijayasundaramNumericalFlux::numerical_flux(double result[4], double w_L[4], double w_R[4],
  double nx, double ny)
{
  error("Not done yet.");
}

double VijayasundaramNumericalFlux::numerical_flux_i(int component, double w_L[4], double w_R[4],
  double nx, double ny)
{
  error("Not done yet.");
  return 0.0;
}

StegerWarmingNumericalFlux::StegerWarmingNumericalFlux(double kappa) : NumericalFlux(kappa) {};


void StegerWarmingNumericalFlux::numerical_flux(double result[4], double w_L[4], double w_R[4],
  double nx, double ny)
{
  double result_temp[4];
  double w_L_temp[4];
  w_L_temp[0] = w_L[0];
  w_L_temp[1] = w_L[1];
  w_L_temp[2] = w_L[2];
  w_L_temp[3] = w_L[3];
  double w_R_temp[4];
  w_R_temp[0] = w_R[0];
  w_R_temp[1] = w_R[1];
  w_R_temp[2] = w_R[2];
  w_R_temp[3] = w_R[3];
  P_plus(result_temp, w_L, w_L_temp, nx, ny);
  P_minus(result, w_R, w_R_temp, nx, ny);
  for(unsigned int i = 0; i < 4; i++)
    result[i] += result_temp[i];
}

double StegerWarmingNumericalFlux::numerical_flux_i(int component, double w_L[4], double w_R[4],
  double nx, double ny)
{
  double result[4];
  numerical_flux(result, w_L, w_R, nx, ny);
  return result[component];
}

void StegerWarmingNumericalFlux::P_plus(double result[4], double w[4], double param[4],
  double nx, double ny)
{
  Q(q, w, nx, ny);

  // Initialize the matrices.
  double T[4][4];
  for(unsigned int i = 0; i < 4; i++)
    for(unsigned int j = 0; j < 4; j++)
      T[i][j] = 0.0;

  double T_inv[4][4];
  for(unsigned int i = 0; i < 4; i++)
    for(unsigned int j = 0; j < 4; j++)
      T_inv[i][j] = 0.0;

  // Calculate A_plus.
  Lambda_plus(result, nx, ny);

  // Calculate the necessary rows / columns of T(T_inv).
  if(result[0] > 0) {
    T_1(T, nx, ny);
    T_inv_1(T_inv, nx, ny);
  }
  if(result[1] > 0) {
    T_2(T, nx, ny);
    T_inv_2(T_inv, nx, ny);
  }
  if(result[2] > 0) {
    T_3(T, nx, ny);
    T_inv_3(T_inv, nx, ny);
  }
  if(result[3] > 0) {
    T_4(T, nx, ny);
    T_inv_4(T_inv, nx, ny);
  }

  // The matrix T * Lambda * T^{-1}
  double diag_inv[4][4];
  double A_1[4][4];
  for(unsigned int i = 0; i < 4; i++)
    for(unsigned int j = 0; j < 4; j++)
      diag_inv[i][j] = result[i] * T_inv[i][j];
  for(unsigned int i = 0; i < 4; i++)
    for(unsigned int j = 0; j < 4; j++) {
      A_1[i][j] = 0;
      for(unsigned int k = 0; k < 4; k++)
        A_1[i][j] += T[i][k] * diag_inv[k][j];
    }

    // Finale.
    Q(param, param, nx, ny);
    for(unsigned int i = 0; i < 4; i++) {
      result[i] = 0;
      for(unsigned int j = 0; j < 4; j++)
        result[i] +=A_1[i][j] * param[j];
    }
    Q_inv(result, result, nx, ny);
}

void StegerWarmingNumericalFlux::P_minus(double result[4], double w[4], double param[4],
  double nx, double ny)
{
  Q(q, w, nx, ny);

  // Initialize the matrices.
  double T[4][4];
  for(unsigned int i = 0; i < 4; i++)
    for(unsigned int j = 0; j < 4; j++)
      T[i][j] = 0.0;

  double T_inv[4][4];
  for(unsigned int i = 0; i < 4; i++)
    for(unsigned int j = 0; j < 4; j++)
      T_inv[i][j] = 0.0;

  // Calculate A_plus.
  Lambda_minus(result, nx, ny);

  // Calculate the necessary rows / columns of T(T_inv).
  if(result[0] < 0) {
    T_1(T, nx, ny);
    T_inv_1(T_inv, nx, ny);
  }
  if(result[1] < 0) {
    T_2(T, nx, ny);
    T_inv_2(T_inv, nx, ny);
  }
  if(result[2] < 0) {
    T_3(T, nx, ny);
    T_inv_3(T_inv, nx, ny);
  }
  if(result[3] < 0) {
    T_4(T, nx, ny);
    T_inv_4(T_inv, nx, ny);
  }


  // The matrix T * Lambda * T^{-1}
  double diag_inv[4][4];
  double A_1[4][4];
  for(unsigned int i = 0; i < 4; i++)
    for(unsigned int j = 0; j < 4; j++)
      diag_inv[i][j] = result[i] * T_inv[i][j];
  for(unsigned int i = 0; i < 4; i++)
    for(unsigned int j = 0; j < 4; j++) {
      A_1[i][j] = 0;
      for(unsigned int k = 0; k < 4; k++)
        A_1[i][j] += T[i][k] * diag_inv[k][j];
    }

    // Finale.
    Q(param, param, nx, ny);
    for(unsigned int i = 0; i < 4; i++) {
      result[i] = 0;
      for(unsigned int j = 0; j < 4; j++)
        result[i] +=A_1[i][j] * param[j];
    }
    Q_inv(result, result, nx, ny);
}

void StegerWarmingNumericalFlux::Lambda_plus(double result[4], double nx, double ny)
{
  a = QuantityCalculator::calc_sound_speed(q[0], q[1], q[2], q[3], kappa);
  u = q[1] / q[0];
  v = q[2] / q[0];
  V = u*u + v*v;
  result[0] = u - a < 0 ? 0 : u - a;
  result[1] = u < 0 ? 0 : u;
  result[2] = u < 0 ? 0 : u;
  result[3] = u + a < 0 ? 0 : u + a;
}

void StegerWarmingNumericalFlux::Lambda_minus(double result[4], double nx, double ny)
{
  a = QuantityCalculator::calc_sound_speed(q[0], q[1], q[2], q[3], kappa);
  u = q[1] / q[0];
  v = q[2] / q[0];
  V = u*u + v*v;
  result[0] = u - a < 0 ? u - a : 0;
  result[1] = u < 0 ? u : 0;
  result[2] = u < 0 ? u : 0;
  result[3] = u + a < 0 ? u + a : 0;
}

void StegerWarmingNumericalFlux::T_1(double result[4][4], double nx, double ny)
{
  result[0][0] = 1.0;
  result[1][0] = u - a;
  result[2][0] = v;
  result[3][0] = (V / 2.0) + (a*a / (kappa - 1.0)) - (u * a);
}
void StegerWarmingNumericalFlux::T_2(double result[4][4], double nx, double ny)
{
  result[0][1] = 1.0;
  result[1][1] = u;
  result[2][1] = v;
  result[3][1] = V / 2.0;
}
void StegerWarmingNumericalFlux::T_3(double result[4][4], double nx, double ny)
{
  result[0][2] = 1.0;
  result[1][2] = u;
  result[2][2] = v - a;
  result[3][2] = (V / 2.0)  - v * a;
}
void StegerWarmingNumericalFlux::T_4(double result[4][4], double nx, double ny)
{
  result[0][3] = 1.0;
  result[1][3] = u + a;
  result[2][3] = v;
  result[3][3] = (V / 2.0) + (a * a / (kappa - 1.0)) + (u * a);
}

void StegerWarmingNumericalFlux::T_inv_1(double result[4][4], double nx, double ny)
{
  result[0][0] = (1.0 / (a * a)) * (0.5 * (((kappa - 1) * V / 2.0) + u * a));
  result[0][1] = (1.0 / (a * a)) * (- (a + u * (kappa - 1.0)) / 2.0);
  result[0][2] = (1.0 / (a * a)) * (- (v * (kappa - 1.0)) / 2.0);
  result[0][3] = (1.0 / (a * a)) * (kappa - 1.0) / 2.0;
}
void StegerWarmingNumericalFlux::T_inv_2(double result[4][4], double nx, double ny)
{
  result[1][0] = (1.0 / (a * a)) * (a * a - v * a - (kappa - 1.0) * (V / 2.0));
  result[1][1] = (1.0 / (a * a)) * u * (kappa - 1.0);
  result[1][2] = (1.0 / (a * a)) * (a + v * (kappa - 1.0));
  result[1][3] = (1.0 / (a * a)) * (1.0 - kappa);
}
void StegerWarmingNumericalFlux::T_inv_3(double result[4][4], double nx, double ny)
{
  result[2][0] = (1.0 / (a * a)) * v * a;
  result[2][1] = (1.0 / (a * a)) * 0.0;
  result[2][2] = (1.0 / (a * a)) * (-a);
  result[2][3] = (1.0 / (a * a)) * 0.0;
}
void StegerWarmingNumericalFlux::T_inv_4(double result[4][4], double nx, double ny)
{
  result[3][0] = (1.0 / (a * a)) * (0.5 * (((kappa - 1.0) * V / 2.0) - u * a));
  result[3][1] = (1.0 / (a * a)) * (a - u * (kappa - 1.0)) / 2.0;
  result[3][2] = (1.0 / (a * a)) * (- (v * (kappa - 1.0)) / 2.0);
  result[3][3] = (1.0 / (a * a)) * (kappa - 1.0) / 2.0;
}

void StegerWarmingNumericalFlux::numerical_flux_solid_wall(double result[4], double w_L[4], double nx, double ny)
{
  Q(q_L, w_L, nx, ny);
  a_B = QuantityCalculator::calc_sound_speed(q_L[0], q_L[1], q_L[2], q_L[3], kappa) + ((kappa - 1) * q_L[1] / (2 * q_L[0]));
  double rho_B = std::pow(a_B * a_B * q_L[0] / (kappa * QuantityCalculator::calc_pressure(q_L[0], q_L[1], q_L[2], q_L[3], kappa)), (1 / (kappa - 1))) * q_L[0];
  q_R[0] = 0;
  q_R[1] = rho_B * a_B * a_B / kappa;
  q_R[2] = 0;
  q_R[3] = 0;
  Q_inv(result, q_R, nx, ny);
}

double StegerWarmingNumericalFlux::numerical_flux_solid_wall_i(int component, double w_L[4], double nx, double ny)
{
  double result[4];
  numerical_flux_solid_wall(result, w_L, nx, ny);
  return result[component];
}

void StegerWarmingNumericalFlux::numerical_flux_inlet(double result[4], double w_L[4], double w_B[4],
  double nx, double ny)
{
  // At the beginning, rotate the states into the local coordinate system and store the left and right state
  // so we do not have to pass it around.
  Q(q_L, w_L, nx, ny);
  Q(q_B, w_B, nx, ny);

  // Speeds of sound.
  a_L = QuantityCalculator::calc_sound_speed(q_L[0], q_L[1], q_L[2], q_L[3], kappa);
  a_B = QuantityCalculator::calc_sound_speed(q_B[0], q_B[1], q_B[2], q_B[3], kappa);

  if(q_L[1] / q_L[0] > a_L) {// Supersonic inlet - everything is prescribed.
    f_1(result, q_B);
    Q_inv(result, result, nx, ny);
    return;
  }
  else {// Subsonic inlet - only rho_b, v_x, v_y are prescribed, pressure is calculated as follows. The pressure is prescribed always so that one can know if the
    // inlet is subsonic or supersonic.
    double a_1 = a_L + ((kappa - 1) / 2) * (q_L[1] / q_L[0] - q_B[1] / q_B[0]);
    q_1[0] = std::pow(a_1 * a_1 * q_L[0] / (kappa * QuantityCalculator::calc_pressure(q_L[0], q_L[1], q_L[2], q_L[3], kappa)), 1 / (kappa - 1)) * q_L[0];
    q_1[1] = q_1[0] * q_B[1] / q_B[0];
    q_1[2] = q_1[0] * q_L[2] / q_L[0];
    q_1[3] = QuantityCalculator::calc_energy(q_1[0], q_1[1], q_1[2], a_1 * a_1 * q_1[0] / kappa, kappa);
    if(q_B[1] / q_B[0] < 0)
      if(q_B[1] / q_B[0] < a_1) {
        f_1(result, q_B);
        Q_inv(result, result, nx, ny);
        return;
      }
      else
      {
        double a_l_star = (((kappa - 1) / (kappa + 1)) * q_L[1] / q_L[0]) + 2 * a_L / (kappa + 1);
        q_L_star[0] = std::pow(a_l_star / a_L, 2 / (kappa - 1)) * q_L[0];
        q_L_star[1] = a_l_star;
        q_L_star[2] = q_L_star[0] * q_L[2] / q_L[0];
        q_L_star[3] = QuantityCalculator::calc_energy(q_L_star[0], q_L_star[1], q_L_star[2], q_L_star[0] * a_l_star * a_l_star / kappa, kappa);
        double first_f_1[4];
        double second_f_1[4];
        double third_f_1[4];
        f_1(first_f_1, q_B);
        f_1(second_f_1, q_L_star);
        f_1(third_f_1, q_1);
        for(unsigned int i = 0; i < 4; i++)
          result[i] = first_f_1[i] + second_f_1[i] - third_f_1[i];
        Q_inv(result, result, nx, ny);
        return;
      }
    else
      if(q_B[1] / q_B[0] < a_1) {
        f_1(result, q_1);
        Q_inv(result, result, nx, ny);
        return;
      }
      else
      {
        double a_l_star = (((kappa - 1) / (kappa + 1)) * q_L[1] / q_L[0]) + 2 * a_L / (kappa + 1);
        q_L_star[0] = std::pow(a_l_star / a_L, 2 / (kappa - 1)) * q_L[0];
        q_L_star[1] = a_l_star;
        q_L_star[2] = q_L_star[0] * q_L[2] / q_L[0];
        q_L_star[3] = QuantityCalculator::calc_energy(q_L_star[0], q_L_star[1], q_L_star[2], q_L_star[0] * a_l_star * a_l_star / kappa, kappa);
        f_1(result, q_L_star);
        Q_inv(result, result, nx, ny);
        return;
      }
  }
}

double StegerWarmingNumericalFlux::numerical_flux_inlet_i(int component, double w_L[4], double w_B[4],
  double nx, double ny)
{
  double result[4];
  numerical_flux_inlet(result, w_L, w_B, nx, ny);
  return result[component];
}

void StegerWarmingNumericalFlux::numerical_flux_outlet(double result[4], double w_L[4], double pressure, double nx, double ny)
{
  // At the beginning, rotate the states into the local coordinate system and store the left and right state
  // so we do not have to pass it around.
  Q(q_L, w_L, nx, ny);

  double a_L = QuantityCalculator::calc_sound_speed(q_L[0], q_L[1], q_L[2], q_L[3], kappa);

  if(q_L[1] / q_L[0] > a_L) {// Supersonic inlet - everything is prescribed.
    f_1(result, q_L);
    Q_inv(result, result, nx, ny);
    return;
  }
  else
  {
    this->q_B[0] = q_L[0] * std::pow(pressure / QuantityCalculator::calc_pressure(this->q_L[0], this->q_L[1], this->q_L[2], this->q_L[3], kappa), 1 / kappa);
    this->q_B[1] = this->q_B[0] * (q_L[1] / q_L[0] + (2 / (kappa - 1)) * (a_L - std::sqrt(kappa * pressure / q_B[0])));
    this->q_B[2] = this->q_B[0] * this->q_L[2] / this->q_L[0];
    this->q_B[3] = QuantityCalculator::calc_energy(this->q_B[0], this->q_B[1], this->q_B[2], pressure, kappa);
    if(q_B[1] / q_B[0] < QuantityCalculator::calc_sound_speed(this->q_B[0], this->q_B[1], this->q_B[2], this->q_B[3], kappa))
    {
      f_1(result, q_B);
      Q_inv(result, result, nx, ny);
      return;
    }
    else
    {
      double a_l_star = (((kappa - 1) / (kappa + 1)) * q_L[1] / q_L[0]) + 2 * a_L / (kappa + 1);
      q_L_star[0] = std::pow(a_l_star / a_L, 2 / (kappa - 1)) * q_L[0];
      q_L_star[1] = a_l_star;
      q_L_star[2] = q_L_star[0] * q_L[2] / q_L[0];
      q_L_star[3] = QuantityCalculator::calc_energy(q_L_star[0], q_L_star[1], q_L_star[2], q_L_star[0] * a_l_star * a_l_star / kappa, kappa);
      f_1(result, q_L_star);
      Q_inv(result, result, nx, ny);
      return;
    }
  }
}

double StegerWarmingNumericalFlux::numerical_flux_outlet_i(int component, double w_L[4], double pressure, double nx, double ny)
{
  double result[4];
  numerical_flux_outlet(result, w_L, pressure, nx, ny);
  return result[component];
}





OsherSolomonNumericalFlux::OsherSolomonNumericalFlux(double kappa) : NumericalFlux(kappa)
{
}

void OsherSolomonNumericalFlux::numerical_flux(double result[4], double w_L[4], double w_R[4],
  double nx, double ny)
{

  // At the beginning, rotate the states into the local coordinate system and store the left and right state
  // so we do not have to pass it around.
  Q(q_L, w_L, nx, ny);
  Q(q_R, w_R, nx, ny);

  // Decide what we have to calculate.
  // Speeds of sound.
  a_L = QuantityCalculator::calc_sound_speed(q_L[0], q_L[1], q_L[2], q_L[3], kappa);
  a_R = QuantityCalculator::calc_sound_speed(q_R[0], q_R[1], q_R[2], q_R[3], kappa);

  // Check that we can use the following.
  double right_hand_side = 0;
  if((q_L[2] / q_L[0]) - (q_R[2] / q_R[0]) > 0)
    right_hand_side = (q_L[2] / q_L[0] - q_R[2] / q_R[0] > 0) / 2;
  if(a_L + a_R + ((kappa - 1) * (q_L[1] / q_L[0] - q_R[1] / q_R[0]) / 2) <= right_hand_side)
    error("Osher-Solomon numerical flux is not possible to construct according to the table.");

  // Utility numbers.
  this->z_L = (0.5 * (kappa - 1) * q_L[1] / q_L[0]) + a_L;
  this->z_R = (0.5 * (kappa - 1) * q_R[1] / q_R[0]) - a_R;
  this->s_L = QuantityCalculator::calc_pressure(q_L[0], q_L[1], q_L[2], q_L[3], kappa) / std::pow(q_L[0], kappa);
  this->s_R = QuantityCalculator::calc_pressure(q_R[0], q_R[1], q_R[2], q_R[3], kappa) / std::pow(q_R[0], kappa);
  this->alpha = std::pow(s_R / s_L, 1 / (2 * kappa));

  // We always need to calculate q_1, a_1, a_3, as we are going to decide what to return based on this.
  calculate_q_1_a_1_a_3();

  // First column in table 3.4.1 on the page 233 in Feist (2003).
  if(q_R[1] / q_R[0] >= - a_R && q_L[1] / q_L[0] <= a_L)
  {    // First row.
    if(a_1 <= q_1[1] / q_1[0]) {
      calculate_q_L_star();
      f_1(result, q_L_star);
      Q_inv(result, result, nx, ny);
      return;
    }
    // Second row.
    if(0 < q_1[1] / q_1[0] && q_1[1] / q_1[0] < a_1) {
      f_1(result, q_1);
      Q_inv(result, result, nx, ny);
      return;
    }
    // Third row.
    if(-a_3 <= q_1[1] / q_1[0] && q_1[1] / q_1[0] <= 0) {
      calculate_q_3();
      f_1(result, q_3);
      Q_inv(result, result, nx, ny);
      return;
    }
    // Fourth row.
    if(q_1[1] / q_1[0] < -a_3) {
      calculate_q_R_star();
      f_1(result, q_R_star);
      Q_inv(result, result, nx, ny);
      return;
    }
  }

  // Second column in table 3.4.1 on the page 233 in Feist (2003).
  if(q_R[1] / q_R[0] >= - a_R && q_L[1] / q_L[0] > a_L)
  {    // First row.
    if(a_1 <= q_1[1] / q_1[0]) {
      f_1(result, q_L);
      Q_inv(result, result, nx, ny);
      return;
    }
    // Second row.
    if(0 < q_1[1] / q_1[0] && q_1[1] / q_1[0] < a_1) {
      calculate_q_L_star();
      double first_f_1[4];
      double second_f_1[4];
      double third_f_1[4];
      f_1(first_f_1, q_L);
      f_1(second_f_1, q_L_star);
      f_1(third_f_1, q_1);
      for(unsigned int i = 0; i < 4; i++)
        result[i] = first_f_1[i] - second_f_1[i] + third_f_1[i];
      Q_inv(result, result, nx, ny);
      return;
    }
    // Third row.
    if(-a_3 <= q_1[1] / q_1[0] && q_1[1] / q_1[0] <= 0) {
      calculate_q_L_star();
      calculate_q_3();
      double first_f_1[4];
      double second_f_1[4];
      double third_f_1[4];
      f_1(first_f_1, q_L);
      f_1(second_f_1, q_L_star);
      f_1(third_f_1, q_3);
      for(unsigned int i = 0; i < 4; i++)
        result[i] = first_f_1[i] - second_f_1[i] + third_f_1[i];
      Q_inv(result, result, nx, ny);
      return;
    }
    // Fourth row.
    if(q_1[1] / q_1[0] < -a_3) {
      calculate_q_L_star();
      calculate_q_R_star();
      double first_f_1[4];
      double second_f_1[4];
      double third_f_1[4];
      f_1(first_f_1, q_L);
      f_1(second_f_1, q_L_star);
      f_1(third_f_1, q_R_star);
      for(unsigned int i = 0; i < 4; i++)
        result[i] = first_f_1[i] - second_f_1[i] + third_f_1[i];
      Q_inv(result, result, nx, ny);
      return;
    }
  }

  // Third column in table 3.4.1 on the page 233 in Feist (2003).
  if(q_R[1] / q_R[0] < - a_R && q_L[1] / q_L[0] <= a_L)
  {    // First row.
    if(a_1 <= q_1[1] / q_1[0]) {
      calculate_q_R_star();
      calculate_q_L_star();
      double first_f_1[4];
      double second_f_1[4];
      double third_f_1[4];
      f_1(first_f_1, q_R);
      f_1(second_f_1, q_R_star);
      f_1(third_f_1, q_L_star);
      for(unsigned int i = 0; i < 4; i++)
        result[i] = first_f_1[i] - second_f_1[i] + third_f_1[i];
      Q_inv(result, result, nx, ny);
      return;
    }
    // Second row.
    if(0 < q_1[1] / q_1[0] && q_1[1] / q_1[0] < a_1) {
      calculate_q_R_star();
      double first_f_1[4];
      double second_f_1[4];
      double third_f_1[4];
      f_1(first_f_1, q_R);
      f_1(second_f_1, q_R_star);
      f_1(third_f_1, q_1);
      for(unsigned int i = 0; i < 4; i++)
        result[i] = first_f_1[i] - second_f_1[i] + third_f_1[i];
      Q_inv(result, result, nx, ny);
      return;
    }
    // Third row.
    if(-a_3 <= q_1[1] / q_1[0] && q_1[1] / q_1[0] <= 0) {
      calculate_q_R_star();
      calculate_q_3();
      double first_f_1[4];
      double second_f_1[4];
      double third_f_1[4];
      f_1(first_f_1, q_R);
      f_1(second_f_1, q_R_star);
      f_1(third_f_1, q_3);
      for(unsigned int i = 0; i < 4; i++)
        result[i] = first_f_1[i] - second_f_1[i] + third_f_1[i];
      return;
    }
    // Fourth row.
    if(q_1[1] / q_1[0] < -a_3) {
      f_1(result, q_R);
      Q_inv(result, result, nx, ny);
      return;
    }
  }

  // Fourth column in table 3.4.1 on the page 233 in Feist (2003).
  if(q_R[1] / q_R[0] < - a_R && q_L[1] / q_L[0] > a_L)
  {    // First row.
    if(a_1 <= q_1[1] / q_1[0]) {
      calculate_q_R_star();
      double first_f_1[4];
      double second_f_1[4];
      double third_f_1[4];
      f_1(first_f_1, q_L);
      f_1(second_f_1, q_R_star);
      f_1(third_f_1, q_R);
      for(unsigned int i = 0; i < 4; i++)
        result[i] = first_f_1[i] - second_f_1[i] + third_f_1[i];
      Q_inv(result, result, nx, ny);
      return;
    }
    // Second row.
    if(0 < q_1[1] / q_1[0] && q_1[1] / q_1[0] < a_1) {
      calculate_q_R_star();
      calculate_q_L_star();
      double first_f_1[4];
      double second_f_1[4];
      double third_f_1[4];
      double fourth_f_1[4];
      double fifth_f_1[4];
      f_1(first_f_1, q_L);
      f_1(second_f_1, q_R_star);
      f_1(third_f_1, q_R);
      f_1(fourth_f_1, q_L_star);
      f_1(fifth_f_1, q_1);
      for(unsigned int i = 0; i < 4; i++)
        result[i] = first_f_1[i] - second_f_1[i] + third_f_1[i] - fourth_f_1[i] + fifth_f_1[i];
      Q_inv(result, result, nx, ny);
      return;
    }
    // Third row.
    if(-a_3 <= q_1[1] / q_1[0] && q_1[1] / q_1[0] <= 0) {
      calculate_q_R_star();
      calculate_q_L_star();
      calculate_q_3();
      double first_f_1[4];
      double second_f_1[4];
      double third_f_1[4];
      double fourth_f_1[4];
      double fifth_f_1[4];
      f_1(first_f_1, q_L);
      f_1(second_f_1, q_R_star);
      f_1(third_f_1, q_R);
      f_1(fourth_f_1, q_L_star);
      f_1(fifth_f_1, q_3);
      for(unsigned int i = 0; i < 4; i++)
        result[i] = first_f_1[i] - second_f_1[i] + third_f_1[i] - fourth_f_1[i] + fifth_f_1[i];
      Q_inv(result, result, nx, ny);
      return;
    }
    // Fourth row.
    if(q_1[1] / q_1[0] < -a_3) {
      calculate_q_L_star();
      double first_f_1[4];
      double second_f_1[4];
      double third_f_1[4];
      f_1(first_f_1, q_L);
      f_1(second_f_1, q_R);
      f_1(third_f_1, q_L_star);
      for(unsigned int i = 0; i < 4; i++)
        result[i] = first_f_1[i] + second_f_1[i] - third_f_1[i];
      Q_inv(result, result, nx, ny);
      return;
    }
  }
}

void OsherSolomonNumericalFlux::numerical_flux_solid_wall(double result[4], double w_L[4], double nx, double ny)
{
  Q(q_L, w_L, nx, ny);
  a_B = QuantityCalculator::calc_sound_speed(q_L[0], q_L[1], q_L[2], q_L[3], kappa) + ((kappa - 1) * q_L[1] / (2 * q_L[0]));
  double rho_B = std::pow(a_B * a_B * q_L[0] / (kappa * QuantityCalculator::calc_pressure(q_L[0], q_L[1], q_L[2], q_L[3], kappa)), (1 / (kappa - 1))) * q_L[0];
  q_R[0] = 0;
  q_R[1] = rho_B * a_B * a_B / kappa;
  q_R[2] = 0;
  q_R[3] = 0;
  Q_inv(result, q_R, nx, ny);
}

double OsherSolomonNumericalFlux::numerical_flux_solid_wall_i(int component, double w_L[4], double nx, double ny)
{
  double result[4];
  numerical_flux_solid_wall(result, w_L, nx, ny);
  return result[component];
}

void OsherSolomonNumericalFlux::numerical_flux_inlet(double result[4], double w_L[4], double w_B[4],
  double nx, double ny)
{
  // At the beginning, rotate the states into the local coordinate system and store the left and right state
  // so we do not have to pass it around.
  Q(q_L, w_L, nx, ny);
  Q(q_B, w_B, nx, ny);

  // Speeds of sound.
  a_L = QuantityCalculator::calc_sound_speed(q_L[0], q_L[1], q_L[2], q_L[3], kappa);
  a_B = QuantityCalculator::calc_sound_speed(q_B[0], q_B[1], q_B[2], q_B[3], kappa);

  if(q_L[1] / q_L[0] > a_L) {// Supersonic inlet - everything is prescribed.
    f_1(result, q_B);
    Q_inv(result, result, nx, ny);
    return;
  }
  else {// Subsonic inlet - only rho_b, v_x, v_y are prescribed, pressure is calculated as follows. The pressure is prescribed always so that one can know if the
    // inlet is subsonic or supersonic.
    double a_1 = a_L + ((kappa - 1) / 2) * (q_L[1] / q_L[0] - q_B[1] / q_B[0]);
    q_1[0] = std::pow(a_1 * a_1 * q_L[0] / (kappa * QuantityCalculator::calc_pressure(q_L[0], q_L[1], q_L[2], q_L[3], kappa)), 1 / (kappa - 1)) * q_L[0];
    q_1[1] = q_1[0] * q_B[1] / q_B[0];
    q_1[2] = q_1[0] * q_L[2] / q_L[0];
    q_1[3] = QuantityCalculator::calc_energy(q_1[0], q_1[1], q_1[2], a_1 * a_1 * q_1[0] / kappa, kappa);
    if(q_B[1] / q_B[0] < 0)
      if(q_B[1] / q_B[0] < a_1) {
        f_1(result, q_B);
        Q_inv(result, result, nx, ny);
        return;
      }
      else
      {
        double a_l_star = (((kappa - 1) / (kappa + 1)) * q_L[1] / q_L[0]) + 2 * a_L / (kappa + 1);
        q_L_star[0] = std::pow(a_l_star / a_L, 2 / (kappa - 1)) * q_L[0];
        q_L_star[1] = a_l_star;
        q_L_star[2] = q_L_star[0] * q_L[2] / q_L[0];
        q_L_star[3] = QuantityCalculator::calc_energy(q_L_star[0], q_L_star[1], q_L_star[2], q_L_star[0] * a_l_star * a_l_star / kappa, kappa);
        double first_f_1[4];
        double second_f_1[4];
        double third_f_1[4];
        f_1(first_f_1, q_B);
        f_1(second_f_1, q_L_star);
        f_1(third_f_1, q_1);
        for(unsigned int i = 0; i < 4; i++)
          result[i] = first_f_1[i] + second_f_1[i] - third_f_1[i];
        Q_inv(result, result, nx, ny);
        return;
      }
    else
      if(q_B[1] / q_B[0] < a_1) {
        f_1(result, q_1);
        Q_inv(result, result, nx, ny);
        return;
      }
      else
      {
        double a_l_star = (((kappa - 1) / (kappa + 1)) * q_L[1] / q_L[0]) + 2 * a_L / (kappa + 1);
        q_L_star[0] = std::pow(a_l_star / a_L, 2 / (kappa - 1)) * q_L[0];
        q_L_star[1] = a_l_star;
        q_L_star[2] = q_L_star[0] * q_L[2] / q_L[0];
        q_L_star[3] = QuantityCalculator::calc_energy(q_L_star[0], q_L_star[1], q_L_star[2], q_L_star[0] * a_l_star * a_l_star / kappa, kappa);
        f_1(result, q_L_star);
        Q_inv(result, result, nx, ny);
        return;
      }
  }
}

double OsherSolomonNumericalFlux::numerical_flux_inlet_i(int component, double w_L[4], double w_B[4],
  double nx, double ny)
{
  double result[4];
  numerical_flux_inlet(result, w_L, w_B, nx, ny);
  return result[component];
}

void OsherSolomonNumericalFlux::numerical_flux_outlet(double result[4], double w_L[4], double pressure, double nx, double ny)
{
  // At the beginning, rotate the states into the local coordinate system and store the left and right state
  // so we do not have to pass it around.
  Q(q_L, w_L, nx, ny);

  double a_L = QuantityCalculator::calc_sound_speed(q_L[0], q_L[1], q_L[2], q_L[3], kappa);

  if(q_L[1] / q_L[0] > a_L) {// Supersonic inlet - everything is prescribed.
    f_1(result, q_L);
    Q_inv(result, result, nx, ny);
    return;
  }
  else
  {
    this->q_B[0] = q_L[0] * std::pow(pressure / QuantityCalculator::calc_pressure(this->q_L[0], this->q_L[1], this->q_L[2], this->q_L[3], kappa), 1 / kappa);
    this->q_B[1] = this->q_B[0] * (q_L[1] / q_L[0] + (2 / (kappa - 1)) * (a_L - std::sqrt(kappa * pressure / q_B[0])));
    this->q_B[2] = this->q_B[0] * this->q_L[2] / this->q_L[0];
    this->q_B[3] = QuantityCalculator::calc_energy(this->q_B[0], this->q_B[1], this->q_B[2], pressure, kappa);
    if(q_B[1] / q_B[0] < QuantityCalculator::calc_sound_speed(this->q_B[0], this->q_B[1], this->q_B[2], this->q_B[3], kappa))
    {
      f_1(result, q_B);
      Q_inv(result, result, nx, ny);
      return;
    }
    else
    {
      double a_l_star = (((kappa - 1) / (kappa + 1)) * q_L[1] / q_L[0]) + 2 * a_L / (kappa + 1);
      q_L_star[0] = std::pow(a_l_star / a_L, 2 / (kappa - 1)) * q_L[0];
      q_L_star[1] = a_l_star;
      q_L_star[2] = q_L_star[0] * q_L[2] / q_L[0];
      q_L_star[3] = QuantityCalculator::calc_energy(q_L_star[0], q_L_star[1], q_L_star[2], q_L_star[0] * a_l_star * a_l_star / kappa, kappa);
      f_1(result, q_L_star);
      Q_inv(result, result, nx, ny);
      return;
    }
  }
}

double OsherSolomonNumericalFlux::numerical_flux_outlet_i(int component, double w_L[4], double pressure, double nx, double ny)
{
  double result[4];
  numerical_flux_outlet(result, w_L, pressure, nx, ny);
  return result[component];
}

void OsherSolomonNumericalFlux::calculate_q_1_a_1_a_3()
{
  this->a_1 = (z_L - z_R) / (1 + alpha);
  this->q_1[0] = std::pow(a_1 / a_L, 2 / (kappa - 1)) * q_L[0];
  this->q_1[1] = this->q_1[0] * 2 * (z_L - a_1) / (kappa - 1);
  this->q_1[2] = this->q_1[0] * this->q_L[2] / this->q_L[0] ;
  this->q_1[3] = QuantityCalculator::calc_energy(this->q_1[0], this->q_1[1], this->q_1[2], a_1 * a_1 * q_1[0] / kappa, kappa);
  this->a_3 = alpha * a_1;
}

void OsherSolomonNumericalFlux::calculate_q_L_star()
{
  this->a_L_star = 2 * z_L / (kappa + 1);
  this->q_L_star[0] = std::pow(a_L_star / a_L, 2 / (kappa -1 )) * q_L[0];
  this->q_L_star[1] = this->q_L_star[0] * a_L_star;
  this->q_L_star[2] = this->q_1[0] * this->q_L[2] / this->q_L[0] ;
  this->q_L_star[3] = QuantityCalculator::calc_energy(this->q_L_star[0], this->q_L_star[1], this->q_L_star[2], a_L_star * a_L_star * q_L_star[0] / kappa, kappa);
}

void OsherSolomonNumericalFlux::calculate_q_3()
{
  // a_3 already calculated.
  this->q_3[0] = q_1[0] / (alpha * alpha);
  this->q_3[1] = this->q_3[0] * this->q_1[1] / this->q_1[0] ;
  this->q_3[2] = this->q_3[0] * this->q_R[2] / this->q_R[0] ;
  this->q_3[3] = QuantityCalculator::calc_energy(this->q_3[0], this->q_3[1], this->q_3[2], a_3 * a_3 * q_3[0] / kappa, kappa);
}

void OsherSolomonNumericalFlux::calculate_q_R_star()
{
  this->a_R_star = - 2 * z_R / (kappa + 1);
  this->q_R_star[0] = std::pow(a_R_star / a_R, 2 / (kappa -1 )) * q_R[0];
  this->q_R_star[1] = this->q_R_star[0] * -a_R_star;
  this->q_R_star[2] = this->q_1[0] * this->q_R[2] / this->q_R[0] ;
  this->q_R_star[3] = QuantityCalculator::calc_energy(this->q_R_star[0], this->q_R_star[1], this->q_R_star[2], a_R_star * a_R_star * q_R_star[0] / kappa, kappa);
}

double OsherSolomonNumericalFlux::numerical_flux_i(int component, double w_L[4], double w_R[4],
  double nx, double ny)
{
  double result[4];
  numerical_flux(result, w_L, w_R, nx, ny);
  return result[component];
}