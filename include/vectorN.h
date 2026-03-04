#pragma once

#include <bx/math.h>
#include <cmath>
#include <cstdint>
#include <type_traits>

template <uint64_t, uint64_t> class Matrix;

struct Vector3;
class Normal3;
template <typename T>
concept Vector3D = std::is_same_v<T, Vector3> || std::is_same_v<T, Normal3>;

struct Vector2;
class Normal2;
template <typename T>
concept Vector2D = std::is_same_v<T, Vector2> || std::is_same_v<T, Normal2>;

struct Vector3
{
	public:
	float x{0};
	float y{0};
	float z{0};

	auto GetX() const -> float;
	auto GetY() const -> float;
	auto GetZ() const -> float;

	auto Magnitude() const -> float;
	auto SquareMag() const -> float;
	auto Normalize() const -> Normal3;
	auto Dot(const Vector3D auto other) const -> float
	{
		return (this->x * other.GetX())
			   + (this->y * other.GetY())
			   + (this->z * other.GetZ());
	}
	auto Angle(const Vector3D auto other) const -> float
	{
		return std::acos(this->Dot(other)
						 / (this->Magnitude() * other.Magnitude()));
	}
	auto Cross(const Vector3D auto other) const -> Vector3
	{
		return {
			.x = (this->y * other.GetZ()) - (this->z * other.GetY()),
			.y = (this->z * other.GetX()) - (this->x * other.GetZ()),
			.z = (this->x * other.GetY()) - (this->y * other.GetX()),
		};
	}

	explicit operator Matrix<1, 3>();
	explicit operator Matrix<1, 4>();
	operator bx::Vec3();
	operator bx::Vec3() const;

	auto operator+(const Vector3D auto other) const -> Vector3
	{
		return {
			.x = this->x + other.GetX(),
			.y = this->y + other.GetY(),
			.z = this->z + other.GetZ(),
		};
	}
	auto operator-(const Vector3D auto other) const -> Vector3
	{
		return {
			.x = this->x - other.GetX(),
			.y = this->y - other.GetY(),
			.z = this->z - other.GetZ(),
		};
	}
	auto operator+() const -> Vector3;
	auto operator-() const -> Vector3;
	auto operator*(const float value) const -> Vector3
	{
		return {
			.x = this->x * value, .y = this->y * value, .z = this->z * value};
	}
	auto operator/(const float value) const -> Vector3
	{
		return {
			.x = this->x / value, .y = this->y / value, .z = this->z / value};
	}

	private:
	explicit operator Normal3();
};
class Normal3
{
	public:
	Normal3(const Normal3&) = default;
	Normal3(Normal3&&) = default;
	~Normal3() = default;

	auto GetX() const -> float;
	auto GetY() const -> float;
	auto GetZ() const -> float;

	static auto Magnitude() -> float;
	static auto SquareMag() -> float;
	auto Normalize() const -> Normal3;
	auto Dot(const Vector3D auto other) const -> float
	{
		return (this->x * other.GetX())
			   + (this->y * other.GetY())
			   + (this->z * other.GetZ());
	}
	auto Angle(const Vector3D auto other) const -> float
	{
		return std::acos(this->Dot(other) / (other.Magnitude()));
	}
	auto Cross(const Vector3D auto other) const -> Vector3
	{
		return {
			.x = (this->y * other.GetZ()) - (this->z * other.GetY()),
			.y = (this->z * other.GetX()) - (this->x * other.GetZ()),
			.z = (this->x * other.GetY()) - (this->y * other.GetX()),
		};
	}

	explicit operator Vector3();
	explicit operator Matrix<1, 3>();
	explicit operator Matrix<1, 4>();

	auto operator+(const Vector3D auto other) const -> Vector3
	{
		return {
			.x = this->x + other.GetX(),
			.y = this->y + other.GetY(),
			.z = this->z + other.GetZ(),
		};
	}
	auto operator-(const Vector3D auto other) const -> Vector3
	{
		return {
			.x = this->x - other.GetX(),
			.y = this->y - other.GetY(),
			.z = this->z - other.GetZ(),
		};
	}
	auto operator+() const -> Normal3;
	auto operator-() const -> Normal3;
	auto operator*(const float value) const -> Vector3
	{
		return {
			.x = this->x * value, .y = this->y * value, .z = this->z * value};
	}
	auto operator/(const float value) const -> Vector3
	{
		return {
			.x = this->x / value, .y = this->y / value, .z = this->z / value};
	}

	auto operator=(const Normal3&) -> Normal3& = default;
	auto operator=(Normal3&&) -> Normal3& = default;

	static constexpr auto Forward() -> Normal3 { return {0.0f, 0.0f, 1.0f}; }
	static constexpr auto Right() -> Normal3 { return {1.0f, 0.0f, 0.0f}; }
	static constexpr auto Up() -> Normal3 { return {0.0f, 1.0f, 0.0f}; }

	private:
	Normal3() = default;
	Normal3(const float x, const float y, const float z) : x(x), y(y), z(z) { };

	friend auto Vector3::Normalize() const -> Normal3;

	float x{0};
	float y{0};
	float z{0};
};

struct Vector2
{
	float x{};
	float y{};
};
class Normal2
{
	float x{};
	float y{};
};
