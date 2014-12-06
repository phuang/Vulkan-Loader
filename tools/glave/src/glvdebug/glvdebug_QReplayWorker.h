/**************************************************************************
 *
 * Copyright 2014 Valve Software
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 **************************************************************************/
#ifndef GLVDEBUG_QREPLAYWORKER_H
#define GLVDEBUG_QREPLAYWORKER_H

#include <QObject>
#include <QWidget>
#include <QCoreApplication>
#include "glvdebug_view.h"
#include "glvdebug_trace_file_utils.h"
#include "glvreplay_factory.h"

class glvdebug_QReplayWorker : public QObject
{
    Q_OBJECT
public:
    glvdebug_QReplayWorker()
        : m_bPauseReplay(false),
          m_bStopReplay(false),
          m_pView(NULL),
          m_pTraceFileInfo(NULL)
    {
        memset(m_pReplayers, 0, sizeof(glv_replay::glv_trace_packet_replay_library*) * GLV_MAX_TRACER_ID_ARRAY_SIZE);
    }

    virtual ~glvdebug_QReplayWorker()
    {
    }

protected slots:
    void playCurrentTraceFile()
    {
        glvdebug_trace_file_info* pTraceFileInfo = m_pTraceFileInfo;
        glvdebug_trace_file_packet_offsets* pCurPacket;
        unsigned int res;
        glv_replay::glv_trace_packet_replay_library *replayer;
    //    glv_trace_packet_message* msgPacket;

        for (uint64_t i = 0; i < pTraceFileInfo->packetCount; i++)
        {
            QCoreApplication::processEvents();
            if (m_bPauseReplay)
            {
                emit ReplayPaused(i);

                do
                {
                    QCoreApplication::processEvents();
                }
                while (m_bPauseReplay);

                emit ReplayContinued();
            }

            if (m_bStopReplay)
            {
                emit ReplayStopped(i);
                m_pView->output_message("Replay stopped.");
                return;
            }

            pCurPacket = &pTraceFileInfo->pPacketOffsets[i];
            switch (pCurPacket->pHeader->packet_id) {
                case GLV_TPI_MESSAGE:
    //                msgPacket = (glv_trace_packet_message*)pCurPacket->pHeader;
    //                if(msgPacket->type == TLLWarn) {
    //                    s_pView->output_warning(msgPacket->message);
    //                } else if(msgPacket->type == TLLError) {
    //                    s_pView->output_error(msgPacket->message);
    //                } else {
    //                    s_pView->output_message(msgPacket->message);
    //                }
                    break;
                case GLV_TPI_MARKER_CHECKPOINT:
                    break;
                case GLV_TPI_MARKER_API_BOUNDARY:
                    break;
                case GLV_TPI_MARKER_API_GROUP_BEGIN:
                    break;
                case GLV_TPI_MARKER_API_GROUP_END:
                    break;
                case GLV_TPI_MARKER_TERMINATE_PROCESS:
                    break;
                //TODO processing code for all the above cases
                default:
                {
                    if (pCurPacket->pHeader->tracer_id >= GLV_MAX_TRACER_ID_ARRAY_SIZE  || pCurPacket->pHeader->tracer_id == GLV_TID_RESERVED) {
                        m_pView->output_warning(QString("Tracer_id from packet num packet %1 invalid.\n").arg(pCurPacket->pHeader->packet_id));
                        continue;
                    }
                    replayer = m_pReplayers[pCurPacket->pHeader->tracer_id];
                    if (replayer == NULL) {
                        m_pView->output_warning(QString("Tracer_id %1 has no valid replayer.\n").arg(pCurPacket->pHeader->tracer_id));
                        continue;
                    }
                    if (pCurPacket->pHeader->packet_id >= GLV_TPI_BEGIN_API_HERE)
                    {
                        // replay the API packet
                        res = replayer->Replay(pCurPacket->pHeader);
                        if (res != glv_replay::GLV_REPLAY_SUCCESS)
                        {
                            m_pView->output_error(QString("Failed to replay packet_id %1.\n").arg(pCurPacket->pHeader->packet_id));
                        }
                    } else {
                        m_pView->output_error(QString("Bad packet type id=%1, index=%2.\n").arg(pCurPacket->pHeader->packet_id).arg(pCurPacket->pHeader->global_packet_index));
                        //return false;
                    }
                }
            }
        }

//        return true;
    }

public slots:
    void StartReplay()
    {
        m_bPauseReplay = false;
        m_bStopReplay = false;

        emit ReplayStarted();
        playCurrentTraceFile();
        emit ReplayFinished();
    }

    void PauseReplay()
    {
        m_bPauseReplay = true;
    }

    void ContinueReplay()
    {
        m_bPauseReplay = false;
    }

    void StopReplay()
    {
        m_bStopReplay = true;
    }

signals:
    void ReplayStarted();
    void ReplayPaused(uint64_t packetIndex);
    void ReplayContinued();
    void ReplayStopped(uint64_t packetIndex);
    void ReplayFinished();

protected:
    bool m_bPauseReplay;
    bool m_bStopReplay;
    glvdebug_view* m_pView;
    glvdebug_trace_file_info* m_pTraceFileInfo;

    bool load_replayers(glvdebug_trace_file_info* pTraceFileInfo, QWidget* pReplayWidget)
    {
        // Get window handle of the widget to replay into.
        assert(pReplayWidget != NULL);
        unsigned int windowWidth = 800;
        unsigned int windowHeight = 600;
        WId hWindow = pReplayWidget->winId();
        windowWidth = pReplayWidget->geometry().width();
        windowHeight = pReplayWidget->geometry().height();

        // load any API specific driver libraries and init replayer objects
        int debuglevel = 0;
        uint8_t tidApi = GLV_TID_RESERVED;

        // uncomment this to display in a separate window (and then comment out the line below it)
    //    glv_replay::Display disp(windowWidth, windowHeight, 0, false);
        glv_replay::Display disp((glv_window_handle)hWindow, windowWidth, windowHeight);

        for (int i = 0; i < GLV_MAX_TRACER_ID_ARRAY_SIZE; i++)
        {
            m_pReplayers[i] = NULL;
        }

        for (int i = 0; i < pTraceFileInfo->header.tracer_count; i++)
        {
            uint8_t tracerId = pTraceFileInfo->header.tracer_id_array[i];
            tidApi = tracerId;

            const GLV_TRACER_REPLAYER_INFO* pReplayerInfo = &(gs_tracerReplayerInfo[tracerId]);

            if (pReplayerInfo->tracerId != tracerId)
            {
                glv_LogError("Replayer info for TracerId (%d) failed consistency check.\n", tracerId);
                assert(!"TracerId in GLV_TRACER_REPLAYER_INFO does not match the requested tracerId. The array needs to be corrected.");
            }
            else if (pReplayerInfo->needsReplayer == TRUE)
            {
                // Have our factory create the necessary replayer
                m_pReplayers[tracerId] = m_replayerFactory.Create(tracerId);

                if (m_pReplayers[tracerId] == NULL)
                {
                    // replayer failed to be created
                    glv_LogError("Couldn't create replayer for TracerId %d.\n", tracerId);
                }
                else
                {
                    // Initialize the replayer
                    int err = m_pReplayers[tracerId]->Initialize(&disp, debuglevel);
                    if (err) {
                        glv_LogError("Couldn't Initialize replayer for TracerId %d.\n", tracerId);
                        return false;
                    }
                }
            }
        }

        if (tidApi == GLV_TID_RESERVED)
        {
            glv_LogError("No API specified in tracefile for replaying.\n");
            return false;
        }

        return true;
    }

    glv_replay::ReplayFactory m_replayerFactory;
    glv_replay::glv_trace_packet_replay_library* m_pReplayers[GLV_MAX_TRACER_ID_ARRAY_SIZE];

};

#endif // GLVDEBUG_QREPLAYWORKER_H
