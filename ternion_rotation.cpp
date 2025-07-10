#include <cmath>
#include <iostream>
#include <vector>

class Vec3 {
public:
    double x, y, z;
    
    Vec3() : x(0), y(0), z(0) {}
    Vec3(double x, double y, double z) : x(x), y(y), z(z) {}
    
    Vec3 operator+(const Vec3& other) const {
        return Vec3(x + other.x, y + other.y, z + other.z);
    }
    
    Vec3 operator-(const Vec3& other) const {
        return Vec3(x - other.x, y - other.y, z - other.z);
    }
    
    Vec3 operator*(double scalar) const {
        return Vec3(x * scalar, y * scalar, z * scalar);
    }
    
    double dot(const Vec3& other) const {
        return x * other.x + y * other.y + z * other.z;
    }
    
    double magnitude() const {
        return std::sqrt(x * x + y * y + z * z);
    }
    
    void print() const {
        std::cout << "(" << x << ", " << y << ", " << z << ")" << std::endl;
    }
};

class Ternion {
private:
    static constexpr double DEFAULT_EPSILON = 1e-6;
    double epsilon;
    
public:
    double x, y, z;
    
    // Constructors
    Ternion(double epsilon = DEFAULT_EPSILON) : x(0), y(0), z(0), epsilon(epsilon) {}
    Ternion(double x, double y, double z, double epsilon = DEFAULT_EPSILON) 
        : x(x), y(y), z(z), epsilon(epsilon) {}
    
    // Create ternion from axis-angle representation
    // Equation (10): r = (n1*tan(φ/2), n2*tan(φ/2), n3*tan(φ/2))
    static Ternion fromAxisAngle(const Vec3& axis, double angle, double epsilon = DEFAULT_EPSILON) {
        double half_angle = angle / 2.0;
        double tan_half = std::tan(half_angle);
        return Ternion(axis.x * tan_half, axis.y * tan_half, axis.z * tan_half, epsilon);
    }
    
    // Scalar multiplication
    Ternion operator*(double scalar) const {
        return Ternion(x * scalar, y * scalar, z * scalar, epsilon);
    }
    
    // Ternion multiplication (composition of rotations)
    // Equation (6) from the paper
    Ternion operator*(const Ternion& other) const {
        // First compute the basic multiplication
        double new_x = x + other.x + y * other.z - z * other.y;
        double new_y = y + other.y + z * other.x - x * other.z;
        double new_z = z + other.z + x * other.y - y * other.x;
        
        // Compute the scaling factor S
        double s = 1.0 - x * other.x - y * other.y - z * other.z;
        double S = (std::abs(s) < epsilon) ? (1.0 / epsilon) : (1.0 / s);
        
        return Ternion(new_x * S, new_y * S, new_z * S, epsilon);
    }
    
    // Inverse of a ternion
    // Equation (6): inverse is simply negation
    Ternion inverse() const {
        return Ternion(-x, -y, -z, epsilon);
    }
    
    // Apply rotation to a vector
    // Equations (7), (8), (9) from the paper
    Vec3 rotate(const Vec3& v) const {
        // Compute ρ = r1² + r2² + r3²
        double rho = x * x + y * y + z * z;
        
        // Compute b = (1 - ρ)/2 and c = 2/(1 + ρ)
        double b = (1.0 - rho) / 2.0;
        double c = 2.0 / (1.0 + rho);
        
        // Build the transformation matrix R (equation 7)
        // R = [r1² + b    r1*r2 + r3  r1*r3 - r2]
        //     [r1*r2 - r3  r2² + b    r2*r3 + r1]
        //     [r1*r3 + r2  r2*r3 - r1  r3² + b  ]
        
        double R11 = x * x + b;
        double R12 = x * y + z;
        double R13 = x * z - y;
        
        double R21 = x * y - z;
        double R22 = y * y + b;
        double R23 = y * z + x;
        
        double R31 = x * z + y;
        double R32 = y * z - x;
        double R33 = z * z + b;
        
        // Apply the transformation: v' = c * (R * v)
        double new_x = c * (R11 * v.x + R12 * v.y + R13 * v.z);
        double new_y = c * (R21 * v.x + R22 * v.y + R23 * v.z);
        double new_z = c * (R31 * v.x + R32 * v.y + R33 * v.z);
        
        return Vec3(new_x, new_y, new_z);
    }
    
    // Convert to axis-angle representation
    std::pair<Vec3, double> toAxisAngle() const {
        double magnitude = std::sqrt(x * x + y * y + z * z);
        if (magnitude < 1e-8) {
            // Near-zero rotation
            return {Vec3(1, 0, 0), 0.0};
        }
        
        double angle = 2.0 * std::atan(magnitude);
        Vec3 axis(x / magnitude, y / magnitude, z / magnitude);
        return {axis, angle};
    }
    
    // Print the ternion
    void print() const {
        std::cout << "Ternion(" << x << ", " << y << ", " << z << ")" << std::endl;
    }
    
    // Get the rotation angle (magnitude of rotation)
    double getRotationAngle() const {
        return 2.0 * std::atan(std::sqrt(x * x + y * y + z * z));
    }
};

// Utility functions for creating common rotations
namespace TernionUtils {
    // Create rotation around X-axis
    Ternion rotationX(double angle) {
        return Ternion::fromAxisAngle(Vec3(1, 0, 0), angle);
    }
    
    // Create rotation around Y-axis
    Ternion rotationY(double angle) {
        return Ternion::fromAxisAngle(Vec3(0, 1, 0), angle);
    }
    
    // Create rotation around Z-axis
    Ternion rotationZ(double angle) {
        return Ternion::fromAxisAngle(Vec3(0, 0, 1), angle);
    }
    
    // Create identity rotation
    Ternion identity() {
        return Ternion(0, 0, 0);
    }
}

// Example usage and tests
int main() {
    std::cout << "=== Ternion Rotation Implementation ===" << std::endl;
    
    // Test 1: Basic rotation around Z-axis
    std::cout << "\nTest 1: 90-degree rotation around Z-axis" << std::endl;
    Ternion rot_z = TernionUtils::rotationZ(M_PI / 2); // 90 degrees
    rot_z.print();
    
    Vec3 point(1, 0, 0);
    std::cout << "Original point: ";
    point.print();
    
    Vec3 rotated = rot_z.rotate(point);
    std::cout << "Rotated point: ";
    rotated.print();
    
    // Test 2: Composition of rotations
    std::cout << "\nTest 2: Composition of rotations" << std::endl;
    Ternion rot_x = TernionUtils::rotationX(M_PI / 4); // 45 degrees around X
    Ternion rot_y = TernionUtils::rotationY(M_PI / 6); // 30 degrees around Y
    
    // Combine rotations: first X, then Y
    Ternion combined = rot_y * rot_x;
    std::cout << "Combined rotation: ";
    combined.print();
    
    // Apply combined rotation
    Vec3 test_point(1, 1, 0);
    std::cout << "Test point: ";
    test_point.print();
    
    Vec3 result1 = combined.rotate(test_point);
    std::cout << "Result with combined rotation: ";
    result1.print();
    
    // Apply rotations separately (should be the same)
    Vec3 temp = rot_x.rotate(test_point);
    Vec3 result2 = rot_y.rotate(temp);
    std::cout << "Result with separate rotations: ";
    result2.print();
    
    // Test 3: Inverse rotation
    std::cout << "\nTest 3: Inverse rotation" << std::endl;
    Ternion rot_inv = rot_z.inverse();
    Vec3 back_rotated = rot_inv.rotate(rotated);
    std::cout << "Back-rotated point (should be original): ";
    back_rotated.print();
    
    // Test 4: Identity rotation
    std::cout << "\nTest 4: Identity rotation" << std::endl;
    Ternion identity = TernionUtils::identity();
    Vec3 unchanged = identity.rotate(point);
    std::cout << "Point after identity rotation: ";
    unchanged.print();
    
    // Test 5: Large angle rotation (near π)
    std::cout << "\nTest 5: Large angle rotation (170 degrees)" << std::endl;
    Ternion large_rot = TernionUtils::rotationZ(170.0 * M_PI / 180.0);
    large_rot.print();
    
    auto [axis, angle] = large_rot.toAxisAngle();
    std::cout << "Axis: ";
    axis.print();
    std::cout << "Angle (degrees): " << angle * 180.0 / M_PI << std::endl;
    
    return 0;
}