struct Surfel
{
    Surfel() { }

    Surfel(Eigen::Vector3f c_, Eigen::Vector3f u_, Eigen::Vector3f v_,
           Eigen::Vector3f p_, unsigned int rgba_)
        : c(c_), u(u_), v(v_), p(p_), rgba(rgba_) { }

    Eigen::Vector3f c,      // Position of the ellipse center point.
                    u, v,   // Ellipse major and minor axis.
                    p;      // Clipping plane.

    unsigned int    rgba;   // Color.
};