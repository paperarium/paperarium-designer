#include "gltransform.h"
#include "globject.h"
#include "glcomponent.h"

GLTransform::GLTransform(GLObject* go, bool isActive, QVector3D pos, QVector3D rot, QVector3D scale) :
    GLComponent(TRANSFORM, go, isActive),
    local_pos(pos),
    local_rot(rot),
    local_scale(scale) {
    local_qrot = local_qrot.fromEulerAngles(local_rot);
}

GLTransform::~GLTransform() {}

void GLTransform::Reset() {
    local_pos *= 0;
    local_rot *= 0;
    local_scale = {1,1,1};
    isUpdated = false;
}

void GLTransform::SetPos(QVector3D pos) {
    local_pos = pos;
    isUpdated = false;
}

void GLTransform::Translate(QVector3D pos) {
    local_pos += pos;
    isUpdated = false;
}

void GLTransform::TranslateX(float x) {
    local_pos.setX(local_pos.x() + x);
    isUpdated = false;
}

void GLTransform::TranslateY(float y) {
    local_pos.setY(local_pos.y() + y);
    isUpdated = false;
}

void GLTransform::TranslateZ(float z) {
    local_pos.setZ(local_pos.z() + z);
    isUpdated = false;
}

void GLTransform::TranslateLeft(float dist) {
    if (!isUpdated)
        GetWorldMatrix();

    local_pos += world_left * dist;
    isUpdated = false;
}

void GLTransform::TranslateUp(float dist) {
    if (!isUpdated)
        GetWorldMatrix();

    local_pos += world_up * dist;
    isUpdated = false;
}

void GLTransform::TranslateForward(float dist) {
    if (!isUpdated)
        GetWorldMatrix();

    local_pos += world_forward * dist;
    isUpdated = false;
}

void GLTransform::SetRotQ(QQuaternion rot) {
    local_qrot = rot;
    float yaw, pitch, roll;
    local_qrot.getEulerAngles(&pitch, &yaw, &roll);
    local_rot = {pitch, yaw, roll};
    isUpdated = false;
}

void GLTransform::RotateX(float x) {
    local_rot.setX(local_rot.x() + x);
    local_qrot = local_qrot.fromEulerAngles(local_rot);
    isUpdated = false;
}


void GLTransform::RotateY(float y) {
    local_rot.setY(local_rot.y() + y);
    local_qrot = local_qrot.fromEulerAngles(local_rot);
    isUpdated = false;
}

void GLTransform::RotateZ(float z) {
    local_rot.setZ(local_rot.z() + z);
    local_qrot = local_qrot.fromEulerAngles(local_rot);
    isUpdated = false;
}

void GLTransform::RotateQ(QQuaternion rot) {
    float yaw, pitch, roll;
    local_qrot *= rot;
    local_qrot.getEulerAngles(&pitch, &yaw, &roll);
    local_rot = {pitch, yaw, roll};
    isUpdated = false;
}

void GLTransform::RotateAngleAxis(float angle, QVector3D axis) {
    if (!isUpdated)
        GetWorldMatrix();

    float yaw, pitch, roll;
    local_qrot = local_qrot * QQuaternion::fromAxisAndAngle(axis, angle);
    local_qrot.getEulerAngles(&pitch, &yaw, &roll);
    local_rot = {pitch, yaw, roll};
    isUpdated = false;
}

void GLTransform::RotateAxisLeft(float angle) {
    if (!isUpdated)
        GetWorldMatrix();

    RotateAngleAxis(angle, {1,0,0});
}

void GLTransform::RotateAxisUp(float angle) {
    if (!isUpdated)
        GetWorldMatrix();

    RotateAngleAxis(angle, {0,1,0});
}

void GLTransform::RotateAxisForward(float angle) {
    if (!isUpdated)
        GetWorldMatrix();

    RotateAngleAxis(angle, {0,0,1});
}

void GLTransform::RemoveRoll() {
    local_rot.setZ(0);
    isUpdated = false;
}

void GLTransform::SetScale(QVector3D scale) {
    local_scale = scale;
    isUpdated = false;
}

void GLTransform::Focus(QVector3D focus) {
    if (!isUpdated) GetWorldMatrix();

    local_qrot = QQuaternion::fromDirection(local_pos - focus, QVector3D::crossProduct(focus - local_pos, world_forward));
    // calculate new world_left
    GetWorldMatrix();
    local_qrot = QQuaternion::fromDirection(local_pos - focus, QVector3D::crossProduct(focus - local_pos, world_left));
    float yaw, pitch, roll;
    local_qrot.getEulerAngles(&pitch, &yaw, &roll);
    local_rot = {pitch, yaw, roll};
    isUpdated = false;
    RotateAxisForward(-roll);
}

void GLTransform::Orbit(float x, float y, QVector3D focus) {
    if (!isUpdated)
        GetWorldMatrix();

    float dist = local_pos.distanceToPoint(focus);
    local_pos = focus;

    QQuaternion rot;
    float yaw, pitch, roll;
    local_qrot = local_qrot * rot.fromAxisAndAngle({0,1,0}, x);
    local_qrot = local_qrot * rot.fromAxisAndAngle({1,0,0}, y);
    local_qrot.getEulerAngles(&pitch, &yaw, &roll);
    local_rot = {pitch, yaw, roll};

    GetWorldMatrix();

    local_pos -= world_forward * dist;
    isUpdated = false;
}

QVector3D GLTransform::GetPos() const
{
    return local_pos;
}

QVector3D GLTransform::GetRot() const
{
    return local_rot;
}

QVector3D GLTransform::GetScale() const
{
    return local_scale;
}

void GLTransform::Save(QDataStream &stream) {}
void GLTransform::Load(QDataStream &stream) {}
void GLTransform::CleanUp() {}

QMatrix4x4 GLTransform::GetWorldMatrix()
{
    if (!isUpdated)
    {
        world_m.setToIdentity();

        if (glObject != nullptr && glObject->parent != nullptr)
            world_m = glObject->parent->transform->GetWorldMatrix();

        world_m.translate(local_pos);
        world_m.rotate(local_qrot);
        world_m.scale(local_scale);

        world_left = -world_m.column(0).toVector3D().normalized();
        world_up = world_m.column(1).toVector3D().normalized();
        world_forward = -world_m.column(2).toVector3D().normalized();

        isUpdated = true;
    }

    return world_m;
}
