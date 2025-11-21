#include <cmath>
#include <algorithm>

using namespace std;

const double EPSILON = 1e-9;

struct Double {
    double value;

    bool is_equal(Double other) const {
        double difference = value - other.value;
        double maximum = max(fabs(value), fabs(other.value));

        if (maximum > 1) {
            difference /= maximum;
        }

        return fabs(difference) < EPSILON;
    }

    bool is_less(Double other) const {
        return (value < other.value) && !is_equal(other);
    }

    // Операторы для удобства
    Double operator+(const Double& other) const { return Double{value + other.value}; }
    Double operator-(const Double& other) const { return Double{value - other.value}; }
    Double operator*(const Double& other) const { return Double{value * other.value}; }
    Double operator/(const Double& other) const { return Double{value / other.value}; }
};

enum class Kind {
    Line,
    Ray,
    Segment
};

struct Point2D {
    Double x, y;

    Point2D operator+(const Point2D& other) const {
        return Point2D{x + other.x, y + other.y};
    }

    Point2D operator-(const Point2D& other) const {
        return Point2D{x - other.x, y - other.y};
    }

    Point2D operator*(const Double& scalar) const {
        return Point2D{x * scalar, y * scalar};
    }

    Double dot(const Point2D& other) const {
        return x * other.x + y * other.y;
    }

    Double cross(const Point2D& other) const {
        return x * other.y - y * other.x;
    }

    Double length_squared() const {
        return x * x + y * y;
    }

    Double length() const {
        return Double{sqrt(length_squared().value)};
    }

    Double distance(const Point2D& other) const {
        return (*this - other).length();
    }
};

// Вспомогательные функции
Double dot(const Point2D& a, const Point2D& b) {
    return a.dot(b);
}

Double cross(const Point2D& a, const Point2D& b) {
    return a.cross(b);
}

Double distance_point_to_line(const Point2D& p, const Point2D& a, const Point2D& b) {
    Point2D ab = b - a;
    Point2D ap = p - a;
    Double cross_val = cross(ap, ab);
    Double ab_length = ab.length();
    
    if (ab_length.is_equal(Double{0.0})) {
        return p.distance(a);
    }
    
    return Double{fabs(cross_val.value) / ab_length.value};
}

Double distance_point_to_ray(const Point2D& p, const Point2D& a, const Point2D& b) {
    Point2D v = b - a;
    Point2D w = p - a;
    Double dot1 = dot(w, v);
    Double dot2 = dot(v, v);

    if (dot2.is_equal(Double{0.0})) {
        return p.distance(a);
    }

    Double t = dot1 / dot2;

    if (t.is_less(Double{0.0})) {
        return p.distance(a);
    }

    Point2D proj = a + v * t;
    return p.distance(proj);
}

Double distance_point_to_segment(const Point2D& p, const Point2D& a, const Point2D& b) {
    Point2D v = b - a;
    Point2D w = p - a;
    Double dot1 = dot(w, v);
    Double dot2 = dot(v, v);

    if (dot2.is_equal(Double{0.0})) {
        return p.distance(a);
    }

    Double t = dot1 / dot2;

    if (t.is_less(Double{0.0})) {
        return p.distance(a);
    }
    if (t.is_less(Double{1.0}) || t.is_equal(Double{1.0})) {
        Point2D proj = a + v * t;
        return p.distance(proj);
    } else {
        return p.distance(b);
    }
}

struct Geometry;

Double distance_point_to_geometry(const Point2D& p, const Geometry& g);

Double min_double(const Double& a, const Double& b) {
    if (a.is_less(b)) return a;
    return b;
}

struct Geometry {
    Kind kind;
    Point2D begin, end;
    
    Point2D direction() const {
        return end - begin;
    }
};

bool is_param_valid(const Double& t, const Kind& kind) {
    switch (kind) {
        case Kind::Line:
            return true;
        case Kind::Ray:
            return !t.is_less(Double{0.0});
        case Kind::Segment:
            return !t.is_less(Double{0.0}) && !Double{1.0}.is_less(t);
        default:
            return false;
    }
}

Double distance_point_to_geometry(const Point2D& p, const Geometry& g) {
    switch (g.kind) {
        case Kind::Line:
            return distance_point_to_line(p, g.begin, g.end);
        case Kind::Ray:
            return distance_point_to_ray(p, g.begin, g.end);
        case Kind::Segment:
            return distance_point_to_segment(p, g.begin, g.end);
        default:
            return Double{1e18};
    }
}

Double distance(Geometry g1, Geometry g2) {
    Point2D u = g1.direction();
    Point2D v = g2.direction();
    Point2D w0 = g1.begin - g2.begin;

    Double A = dot(u, u);
    Double B = dot(u, v);
    Double C = dot(v, v);
    Double D = dot(u, w0);
    Double E = dot(v, w0);

    Double det = A * C - B * B;

    Double candidate = Double{1e18};

    // Проверяем пересечение в параметрическом пространстве
    if (!det.is_equal(Double{0.0})) {
        Double t = (B * E - C * D) / det;
        Double s = (B * D - A * E) / det;

        if (is_param_valid(t, g1.kind) && is_param_valid(s, g2.kind)) {
            Point2D P = g1.begin + u * t;
            Point2D Q = g2.begin + v * s;
            candidate = min_double(candidate, P.distance(Q));
        }
    }

    // Для параллельных прямых используем расстояние от точки до прямой
    if (g1.kind == Kind::Line && g2.kind == Kind::Line && det.is_equal(Double{0.0})) {
        candidate = min_double(candidate, distance_point_to_geometry(g1.begin, g2));
    }

    // Проверяем граничные точки g1 относительно g2
    candidate = min_double(candidate, distance_point_to_geometry(g1.begin, g2));
    if (g1.kind == Kind::Segment) {
        candidate = min_double(candidate, distance_point_to_geometry(g1.end, g2));
    }

    // Проверяем граничные точки g2 относительно g1
    candidate = min_double(candidate, distance_point_to_geometry(g2.begin, g1));
    if (g2.kind == Kind::Segment) {
        candidate = min_double(candidate, distance_point_to_geometry(g2.end, g1));
    }

    return candidate;
}
