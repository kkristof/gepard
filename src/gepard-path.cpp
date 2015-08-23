/* Copyright (C) 2015, Szilard Ledan <szledan@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies,
 * either expressed or implied, of the FreeBSD Project.
 */

#include "gepard-path.h"
#include <list>
#include <map>
#include <math.h>
#include <memory>

namespace gepard {

void compileShaderProg(GLuint* result, const char* name, const GLchar *vertexShaderSource, const GLchar *fragmentShaderSource)
{
    GLuint shaderProgram = 0;
    GLuint vertexShader = 0;
    GLuint fragmentShader = 0;
    GLint intValue;
    GLchar *log = NULL;
    GLsizei length;

    if (*result)
        return;

    // Shader programs are zero terminated
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &intValue);
    if (intValue != GL_TRUE) {
        glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &intValue);
        log = (GLchar*)malloc(intValue);
        glGetShaderInfoLog(vertexShader, intValue, &length, log);
        printf("Vertex shader compilation failed with: %s\n", log);
        goto error;
    }

    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &intValue);
    if (intValue != GL_TRUE) {
        glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &intValue);
        log = (GLchar*)malloc(intValue);
        glGetShaderInfoLog(fragmentShader, intValue, &length, log);
        printf("Fragment shader compilation failed with: %s\n", log);
        goto error;
    }

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &intValue);
    if (intValue != GL_TRUE) {
        GLchar *log;
        GLsizei length = -13;
        glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &intValue);
        log = (GLchar*)malloc(intValue);
        glGetProgramInfoLog(fragmentShader, intValue, &length, log);
        printf("Shader program link failed with: %s\n", log);
        goto error;
    }

    // According to the specification, the shaders are kept
    // around until the program object is freed (reference counted).
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    *result = shaderProgram;
    return;

error:
    printf("Warning: Shader program cannot be compiled!\n");
    if (log)
        free(log);
    if (vertexShader)
        glDeleteShader(vertexShader);
    if (fragmentShader)
        glDeleteShader(fragmentShader);
    if (shaderProgram)
        glDeleteProgram(shaderProgram);
}

GLuint createFrameBuffer(GLuint texture)
{
    GLuint frameBufferObject;

    // Create a framebuffer object.
    glGenFramebuffers(1, &frameBufferObject);
    if (glGetError() != GL_NO_ERROR) {
        printf("Cannot allocate a frame buffer object\n");
        return 0;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, frameBufferObject);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return frameBufferObject;
}

const GLchar* simpleVertexShader = PROGRAM(

    attribute vec2 a_position;
    attribute vec4 a_color;

    varying vec4 v_color;

    void main(void)
    {
        v_color = a_color;
        gl_Position = vec4(a_position, 1.0, 1.0);
    }
);

const GLchar* simpleFragmentShader = PROGRAM(
    precision mediump float;

    varying vec4 v_color;

    void main(void)
    {
        gl_FragColor = v_color;
    }
);

std::ostream& operator<<(std::ostream& os, const PathElement& ps)
{
    return ps.output(os);
}

void PathData::addMoveToElement(FloatPoint to)
{
    if (_lastElement && _lastElement->isMoveTo()) {
        _lastElement->_to = to;
        return;
    }

    PathElement* currentElement = static_cast<PathElement*>(new (_region.alloc(sizeof(MoveToElement))) MoveToElement(to));

    if (!_firstElement) {
        _firstElement = currentElement;
        _lastElement = _firstElement;
    } else {
        _lastElement->_next = currentElement;
        _lastElement = _lastElement->_next;
    }

    _lastMoveToElement = _lastElement;
}

void PathData::addLineToElement(FloatPoint to)
{
    if (!_lastElement)
        addMoveToElement(to);

    if (!_lastElement->isMoveTo() && _lastElement->_to == to)
        return;

    _lastElement->_next = static_cast<PathElement*>(new (_region.alloc(sizeof(LineToElement))) LineToElement(to));
    _lastElement = _lastElement->_next;
}

void PathData::addQuadaraticCurveToElement(FloatPoint control, FloatPoint to)
{
    if (!_lastElement)
        addMoveToElement(to);

    _lastElement->_next = static_cast<PathElement*>(new (_region.alloc(sizeof(QuadraticCurveToElement))) QuadraticCurveToElement(control, to));
    _lastElement = _lastElement->_next;
}

void PathData::addBezierCurveToElement(FloatPoint control1, FloatPoint control2, FloatPoint to)
{
    if (!_lastElement)
        addMoveToElement(to);

    _lastElement->_next = static_cast<PathElement*>(new (_region.alloc(sizeof(BezierCurveToElement))) BezierCurveToElement(control1, control2, to));
    _lastElement = _lastElement->_next;
}

void PathData::addArcElement(FloatPoint center, FloatPoint radius, Float startAngle, Float endAngle, bool antiClockwise)
{
    if (!_lastElement)
        addMoveToElement(center);

    _lastElement->_next = static_cast<PathElement*>(new (_region.alloc(sizeof(ArcElement))) ArcElement(center, radius, startAngle, endAngle, antiClockwise));
    _lastElement = _lastElement->_next;
}

void PathData::addCloseSubpath()
{
    if (!_lastElement)
        return;
    if (_lastElement->isMoveTo()) {
        addLineToElement(_lastElement->_to);
    }

    _lastElement->_next = static_cast<PathElement*>(new (_region.alloc(sizeof(CloseSubpathElement))) CloseSubpathElement(_lastMoveToElement->_to));
    _lastElement = _lastElement->_next;
}

void PathData::dump()
{
    PathElement* element = _firstElement;

    std::cout << "firstElement: " << _firstElement << std::endl;
    std::cout << "lastElement: " << _lastElement << std::endl;
    std::cout << "lastMoveToElement: " << _lastMoveToElement << std::endl;
    std::cout << "PathData:";
    while (element) {
        std::cout << " " << *element;
        element = element->_next;
    }
    std::cout << std::endl;
}

void Path::fillPath()
{
    if (_pathData._lastElement->_type != PathElementTypes::CloseSubpath)
        _pathData.addCloseSubpath();

    _pathData.dump();
    TrapezoidTessallator tt(this);
    tt.trapezoidList();

#if 0 // unused
    static GLuint simpleShader = 0;
    GLint intValue;
    static GLubyte indicies[] = { 0, 1, 2, 3, 4, 5 };
    static GLfloat allAttributes[] = {
        -1, -1,               // position_1
        1.0, 0.0, 0.0, 0.3,     // color_1
        -0.3, 1,               // position_2
        0.0, 1.0, 0.0, 0.3,     // color_2
        0.4, -1,               // position_3
        0.0, 0.0, 1.0, 0.3,     // color_3
        -0.4, -1,               // position_4
        1.0, 0.0, 0.0, 0.3,     // color_4
        0.3,  1,                // position_5
        0.0, 1.0, 0.0, 0.3,     // color_5
        1, -1,                  // position_6
        0.0, 0.0, 1.0, 0.3,     // color_6
    };

    compileShaderProg(&simpleShader, "SimpleShader", simpleVertexShader, simpleFragmentShader);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(simpleShader);

    /* Triangle 1 and 2 */
    intValue = glGetAttribLocation(simpleShader, "a_position");
    glEnableVertexAttribArray(intValue);
    glVertexAttribPointer(intValue, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(GL_FLOAT), allAttributes);

    intValue = glGetAttribLocation(simpleShader, "a_color");
    glEnableVertexAttribArray(intValue);
    glVertexAttribPointer(intValue, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(GL_FLOAT), allAttributes + 2);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, indicies);
    eglSwapBuffers(_surface->eglDisplay(), _surface->eglSurface());
#endif
}

/* SegmentApproximator */

void SegmentApproximator::insertSegment(Segment segment)
{
    if (segment._direction == Segment::EqualOrNonExist)
        return;

    Float topY = segment.topY();
    Float bottomY = segment.bottomY();

    if (_segments.find(topY) == _segments.end())
    {
        _segments.emplace(topY, new SegmentList()).first->second->push_front(segment);
        // TODO: log: std::cout << *(_segments[topY].begin()) << std::endl;
    } else {
        _segments[topY]->push_front(segment);
    }

    if (_segments.find(bottomY) == _segments.end())
    {
        _segments.emplace(bottomY, new SegmentList());
    }
}

void SegmentApproximator::insertQuadCurve(FloatPoint from, FloatPoint control, FloatPoint to)
{
    // FIXME: Wrong approximation!
    insertSegment(Segment(from, control));
    insertSegment(Segment(control, to));
}

void SegmentApproximator::insertBezierCurve(FloatPoint from, FloatPoint control1, FloatPoint control2, FloatPoint to)
{
    // FIXME: Wrong approximation!
    insertSegment(Segment(from, control1));
    insertSegment(Segment(control1, control2));
    insertSegment(Segment(control2, to));
}

void SegmentApproximator::insertArc(FloatPoint center, FloatPoint radius, float startAngle, float endAngle, bool antiClockwise)
{
    // TODO: Missing approximation!
}

void SegmentApproximator::printSegements()
{
    for (auto& currentSegments : _segments) {
        std::cout << currentSegments.first << ": ";
        SegmentList currentList = *(currentSegments.second);
        for (auto segment : currentList)
            std::cout << segment << " ";
        std::cout << std::endl;
    }
}

inline void SegmentApproximator::splitSegments()
{
    for (SegmentTree::iterator currentSegments = _segments.begin(); currentSegments != _segments.end(); currentSegments++) {
        SegmentTree::iterator newSegments = currentSegments;
        newSegments++;
        if (newSegments != _segments.end()) {
            SegmentList*  currentList = currentSegments->second;
            SegmentList* newList = newSegments->second;
            Float splitY = newSegments->first;
            ASSERT(currentList && newList);

            for (auto& segment : *currentList) {
                if (segment.isOnSegment(splitY))
                    newList->push_front(segment.splitSegment(splitY));
            }
        }
    }
}

SegmentList* SegmentApproximator::segments()
{
    // 1. Split segments with all y lines.
    splitSegments();

    // 2. Find all intersection points.
    for (auto& currentSegments : _segments) {
        SegmentList* currentList = currentSegments.second;
        SegmentList::iterator currentSegment = currentList->begin();
        for (SegmentList::iterator segment = currentSegment; currentSegment != currentList->end(); segment++) {
            if (segment != currentList->end()) {
                Float y = currentSegment->computeIntersectionY(&(*segment));
                if (y != NAN && y != INFINITY)
                    _segments.emplace(y, new SegmentList());
            } else {
                segment = ++currentSegment;
            }
        }
    }

    // 3. Split segments with all y lines.
    splitSegments();

    // 4. Merge and sort all segment list.
    SegmentList* segments = new SegmentList();
    for (SegmentTree::iterator currentSegments = _segments.begin(); currentSegments != _segments.end(); currentSegments++) {
        SegmentList* currentList = currentSegments->second;
        currentList->sort();
        segments->merge(*currentList);
    }

    // 5. Return independent segments.
    return segments;
}

/* TrapezoidTessallator */

TrapezoidList TrapezoidTessallator::trapezoidList()
{
    PathElement* element = _path->pathData()._firstElement;

    if (!element)
        return TrapezoidList();

    ASSERT(element->_type == PathElementTypes::MoveTo);

    SegmentApproximator segmentApproximator;
    FloatPoint from;
    FloatPoint to = element->_to;
    FloatPoint lastMoveTo = to;

    // 1. Insert path elements.
    do {
        from = to;
        element = element->_next;
        to = element->_to;
        switch (element->_type) {
        case PathElementTypes::MoveTo: {
            segmentApproximator.insertSegment(from, lastMoveTo);
            lastMoveTo = to;
            break;
        }
        case PathElementTypes::LineTo: {
            segmentApproximator.insertSegment(from, to);
            break;
        }
        case PathElementTypes::CloseSubpath: {
            segmentApproximator.insertSegment(from, lastMoveTo);
            lastMoveTo = to;
            break;
        }
        case PathElementTypes::QuadraticCurve: {
            // FIXME: Implement real curve, it's only a test solution.
            QuadraticCurveToElement* qe = reinterpret_cast<QuadraticCurveToElement*>(element);
            segmentApproximator.insertQuadCurve(from, qe->_control, to);
            break;
        }
        case PathElementTypes::BezierCurve: {
            // FIXME: Implement real curve, it's only a test solution.
            BezierCurveToElement* be = reinterpret_cast<BezierCurveToElement*>(element);
            segmentApproximator.insertBezierCurve(from, be->_control1, be->_control2, to);
            break;
        }
        case PathElementTypes::Arc: {
            // FIXME: Not implemented!
            break;
        }
        default:
            // unreachable
            break;
        }
    } while (element->_next);

    segmentApproximator.insertSegment(lastMoveTo, element->_to);

    // 2. Use approximator to generate the list of segments.
    std::cout << "SegmentList: ";
    SegmentList* currentList = segmentApproximator.segments();
    for (auto segment : *currentList)
        std::cout << segment << " ";
    std::cout << std::endl;
    if (currentList)
        delete currentList;

    // TODO: 3. Generate trapezoids.
    TrapezoidList trapezoidList;
    return trapezoidList;
}

} // namespace gepard
